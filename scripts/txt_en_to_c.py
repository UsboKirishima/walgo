def txt_to_c_array(txt_path, array_name="raw_en_wordlist"):
    with open(txt_path, "r", encoding="utf-8") as f:
        words = [line.strip() for line in f if line.strip()]  

    c_lines = [f'const char *{array_name}[] = {{']
    for i, word in enumerate(words):
        comma = "," if i < len(words) - 1 else ""
        c_lines.append(f'    "{word}"{comma}')
    c_lines.append("};")

    return "\n".join(c_lines)


if __name__ == "__main__":
    path = "5000-more-common.txt"
    result = txt_to_c_array(path)

    with open("en_wordlist.def", "w", encoding="utf-8") as out:
        out.write(result + "\n")

    print("✅ Generated file en_wordlist.def")

