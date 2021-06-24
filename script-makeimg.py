import os
import sys

while (True):
    os_file = input("os(.bin):")
    fos_file_w = open(os_file, "rb")
    if not fos_file_w:
        print(os_file, " does not exits")
        continue
    os_file_content = fos_file_w.read()
    size_os_file = os.path.getsize(os_file)

    ipl_file = input("ipl(.bin):")
    fipl_file_w = open(ipl_file, "rb")

    if not fipl_file_w:
        print(fipl_file_w, " does not exits")
        continue
    ipl_file_content = fipl_file_w.read()
    size_ipl_file = os.path.getsize(ipl_file)
    break

os_file_content += ipl_file_content
os_file_content += (b'\x00' * (1474560 - size_ipl_file - size_os_file))
try:
    out_file = input("img(.img)")
    with open(out_file, "wb") as f:
        f.write(os_file_content)
except Exception as e:
    print("file type error")
