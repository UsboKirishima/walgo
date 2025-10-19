import json

def json_to_c_array(json_path, array_name="raw_en_wordlist"):
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    words = list(data.keys())

    c_lines = [f'const char *{array_name}[] = {{']
    for i, word in enumerate(words):
        comma = "," if i < len(words) - 1 else ""
        c_lines.append(f'    "{word}"{comma}')
    c_lines.append("};")

    return "\n".join(c_lines)


if __name__ == "__main__":
    path = "words_dictionary.json"
    result = json_to_c_array(path)

    with open("words.en.def", "w", encoding="utf-8") as out:
        out.write(result + "\n")

    print("✅ Generated file")

