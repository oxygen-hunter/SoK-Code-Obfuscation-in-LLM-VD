def get_txt_content_as_str(file_dir)->str:
    with open(file_dir, 'r', encoding='utf-8') as f:
        content = f.read()

    return content

def get_txt_content_as_lines(file_dir)->list:
    with open(file_dir, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    return lines