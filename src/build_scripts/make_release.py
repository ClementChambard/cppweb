import subprocess


def compress_css(css_code):
    # TODO: compress css
    return css_code


def compress_resource(input_file_name, output_file_name):
    if input_file_name.endswith(".css"):
        file_content = ""
        with open(input_file_name, "r") as f:
            file_content = f.read()
        file_content = compress_css(file_content)
        with open(output_file_name, "w") as f:
            f.write(file_content)
    else:
        subprocess.run(["cp", input_file_name, output_file_name])
    if not output_file_name.endswith(".gz"):
        subprocess.run(["gzip", "-f", output_file_name])
        output_file_name += ".gz"
    print(f"--> Compressed resource: {input_file_name} -> {output_file_name}")


def get_files_recurse(dir_name):
    from os import walk

    f = []
    dirs = []
    for _, dirnames, filenames in walk(dir_name):
        f.extend([dir_name + "/" + f for f in filenames])
        for d in dirnames:
            files, ds = get_files_recurse(dir_name + "/" + d)
            f.extend(files)
            dirs.extend(ds)
        break
    if len(f) > 0:
        dirs.append(dir_name)
    return f, dirs


def make_release_resources(resource_dir_name, release_resource_dir_name):
    files, dirs = get_files_recurse(resource_dir_name)
    for d in dirs:
        d = release_resource_dir_name + d[len(resource_dir_name) :]
        subprocess.run(["mkdir", "-p", d])
    for f in files:
        input_file = f
        output_file = release_resource_dir_name + f[len(resource_dir_name) :]
        compress_resource(input_file, output_file)


def copy_resources(resource_dir_name, release_resource_dir_name):
    files, dirs = get_files_recurse(resource_dir_name)
    for d in dirs:
        d = release_resource_dir_name + d[len(resource_dir_name) :]
        subprocess.run(["mkdir", "-p", d])
    for f in files:
        input_file = f
        output_file = release_resource_dir_name + f[len(resource_dir_name) :]
        subprocess.run(["cp", input_file, output_file])
        print(f"--> Copied resource: {input_file} -> {output_file}")


def get_html_files_to_compile(root_dir_name):
    dirs = set()
    files = set()
    lines = []
    with open(root_dir_name + "/required.txt", "r") as f:
        lines = f.readlines()
    for line in lines:
        line = line.strip()
        html_name = line.replace("::", "/") + ".html"
        if line.endswith("*"):
            f, d = get_files_recurse((root_dir_name + "/" + html_name)[:-7])  # *.html
            files.update(f)
            dirs.update(d)
            continue
        file_name = root_dir_name + "/" + html_name
        files.add(file_name)
        dir_name = "/".join(file_name.split("/")[:-1])
        dirs.add(dir_name)
    return list(files), list(dirs)


def make_html_files(html_dir_name, release_html_dir_name):
    files, dirs = get_html_files_to_compile(html_dir_name)
    for d in dirs:
        d = release_html_dir_name + d[len(html_dir_name) :]
        subprocess.run(["mkdir", "-p", d])
    subprocess.run(["mkdir", "-p", release_html_dir_name + "/tmp"])
    for f in files:
        input_file = f[len(html_dir_name) + 1 :]
        from py_pkg import compile_html as COMPILE

        COMPILE(html_dir_name, release_html_dir_name, input_file)

        print(f"--> Compiled html: {input_file}")


RELEASE_DIR = "build/release"


def main():
    make_release_resources("public", f"{RELEASE_DIR}/public")
    copy_resources("private", f"{RELEASE_DIR}/private")
    make_html_files("html", f"{RELEASE_DIR}/html")


if __name__ == "__main__":
    main()
