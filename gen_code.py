#!/usr/bin/env python3
from subprocess import check_output
from dataclasses import dataclass

import ctypes, ctypes.util
whereislib = ctypes.util.find_library('c')
clib = ctypes.cdll.LoadLibrary(whereislib)

UNKNOWN = 33

@dataclass
class Data:
    mask: int
    idx: int
    table_size: int
    enum: int
    string: str

def _parse_int(s):
    s = s.strip()
    if s.startswith("0x"):
        return int(s, 16)
    else:
        return int(s)

def _parse(l):
    len_, *stripped, string = l.strip().split()
    mask, idx, table_size, enum = map(_parse_int, map(str.strip, stripped))

    return Data(mask, idx, table_size, enum, string)

def _strings_to_buffers(arr, l):
    assert all(len(s) == l or (len(s) == 0) for s in arr) 

    rows = ["{" + ", ".join(map("'{}'".format, s)) + "}" for s in arr]
    code = f"static char lookup[{len(arr)}][{l}] = " + "{" + ",\n".join(rows) + "};"
    return code

def _vals_to_buffers(arr, l):
    code = f"static uint8_t value[{len(arr)}] = " + "{" + ", ".join(map(str, arr)) + "};"
    return code


if __name__ == "__main__":
    raw = check_output("./find_mask").decode('utf-8').strip()
    parsed = [_parse(l) for l in raw.split("\n")]
    
    data_by_str = {data.string: data for data in parsed}
    mask_by_len = {len(data.string): (data.mask, data.table_size) for data in parsed}
    strings_by_len = {}
    for d in parsed:
        strings_by_len[len(d.string)] = strings_by_len.get(len(d.string), []) + [d.string]

    inner_code = ""

    for l, vals in strings_by_len.items():
        mask, table_size = mask_by_len[l]
        strings, enums = [""] * table_size, [UNKNOWN] * table_size

        for s in vals:
            strings[data_by_str[s].idx] = s
            enums[data_by_str[s].idx] = data_by_str[s].enum

        inner_code += f"""
            case {l}: {{
                {_strings_to_buffers(strings, l)}
                {_vals_to_buffers(enums, l)}
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), {min(l, 4)});

                uint64_t idx = _pext_u32(as_uint, {mask});
                if (std::memcmp(lookup[idx], s.data(), {l}) == 0) {{
                    return (verb) value[idx];
                }}
                break;
            }}

        """

    code = f"""
    #include "http_verb.h"

    namespace pext_split {{
        using ::boost::beast::http::verb;

        verb
        string_to_verb(std::string_view s) {{
            switch(s.size()) {{
            case 0: case 1: case 2:
                break;
            {inner_code}
            default:
                break;
            }}

            return verb::unknown;
        }}
    }} // namespace pext_split
    """
    print(code)

