#!/usr/bin/env python3

# Tool for automatically updating Conan packages to the latest release
# This script requires the lastversion and filehash python scripts to be installed
# pip install lastversion filehash

from filehash import FileHash
from glob import glob
from lastversion import latest
from packaging import version
from urllib.request import urlretrieve
import os
import yaml


def update_pkg(repo, folder):
    path = os.path.join(folder, "conandata.yml")
    with open(path, "r") as file:
        conandata = yaml.load(file, Loader=yaml.FullLoader)
    v = sorted(conandata["sources"].items(), reverse=True)[0][0]
    latest_version = latest(repo=repo, output_format="dict")
    if latest_version["version"] > version.parse(v):
        print(f'{repo} has newer version: {latest_version["version"]} (from: {v})')
        url = f'https://github.com/{repo}/archive/refs/tags/{latest_version["tag_name"]}.tar.gz'
        tmp_path, headers = urlretrieve(url)
        hs = FileHash("sha256")
        hash = hs.hash_file(tmp_path)
        conandata["sources"][str(latest_version["version"])] = {
            "url": url,
            "sha256": hash,
        }
        print(f'Added new data: "{url}" "{hash}"')
        with open(path, "w") as ofile:
            yaml.dump(conandata, ofile, default_flow_style=False)
    else:
        print(f"{repo} has no update available")


dirs = glob("./*/")
for p in dirs:
    path = os.path.join(p, "repoinfo.yml")
    if not os.path.isfile(path):
        continue
    with open(path) as file:
        repoinfo = yaml.load(file, Loader=yaml.FullLoader)
        for i in repoinfo:
            update_pkg(i["repo"], os.path.join(p, i["folder"]))
