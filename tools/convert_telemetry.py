#!/usr/bin/env python3

from argparse import ArgumentParser, Namespace
import re

def parse_args() -> Namespace:
    parser = ArgumentParser()
    parser.add_argument('-i', '--input', required=True, help='Telemetry .h file')
    parser.add_argument('-o', '--output', required=True, help='Javascript output file')
    return parser.parse_args()

def parse_header_file(input_file: str):
    struct_pattern = re.compile(r"typedef\s+struct\s*\{([^}]*)\}\s*__attribute__\(\(packed\)\)\s*(\w+);", re.DOTALL)
    member_pattern = re.compile(r"(\w+_t|\w+)\s+(\w+);")

    with open(input_file, 'r') as file:
        content = file.read()

    struct_match = struct_pattern.search(content)
    if not struct_match:
        raise ValueError("No struct found in the header file")

    members = member_pattern.findall(struct_match.group(1))
    struct_name = struct_match.group(2)

    return struct_name, members

def get_js_type_and_size(c_type: str):
    return ('number', 4, 'Float32')

def generate_js_class(struct_name, members):
    js_class = f"class Telemetry {{\n"
    js_class += "    constructor() {\n"

    for c_type, name in members:
        js_type, _, _ = get_js_type_and_size(c_type)
        js_class += f"        this.{name} = 0;  // {js_type}\n"

    js_class += "    }\n\n"

    total_size = sum(get_js_type_and_size(c_type)[1] for c_type, _ in members)
    js_class += "    static size() {\n"
    js_class += f"        return {total_size};\n"
    js_class += "    }\n\n"

    js_class += "    static fromBytes(bytes) {\n"
    js_class += "        if (bytes.byteLength !== this.size()) {\n"
    js_class += "            throw new Error('Invalid byte array length');\n"
    js_class += "        }\n"
    js_class += "        let offset = 0;\n"
    js_class += "        let telemetry = new Telemetry();\n"
    js_class += "        const dataView = new DataView(bytes);\n"

    for c_type, name in members:
        _, size, read_method = get_js_type_and_size(c_type)
        js_class += f"        telemetry.{name} = dataView.get{read_method}(offset, true);\n"
        js_class += f"        offset += {size};\n"

    js_class += "        return telemetry;\n"
    js_class += "    }\n"
    js_class += "}\n"
    js_class += "\n"
    js_class += "export default Telemetry;\n"

    return js_class

def main(input_file: str, output_file: str) -> None:
    struct_name, members = parse_header_file(input_file)
    js_class = generate_js_class(struct_name, members)

    with open(output_file, 'w') as file:
        file.write(js_class)

if __name__ == '__main__':
    args = parse_args()
    main(args.input, args.output)
    print(f'[TELEMETRY GENERATOR] Generated telemetry js file at: {args.output}')
