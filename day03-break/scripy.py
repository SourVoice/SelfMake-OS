import os
import sys

os_file = input("os(.bin):")
fos_file_w = open(os_file, "rb")
os_file_content = fos_file_w.read()
size_os_file = os.path.getsize(os_file)

ipl_file = input("ipl(.bin):")
fipl_file_w = open(ipl_file, "rb")
ipl_file_content = fipl_file_w.read()
size_ipl_file = os.path.getsize(ipl_file)

os_file_content += ipl_file_content
os_file_content += (b'\x00' * (1474560 - size_ipl_file - size_os_file))
out_file = input("img(.img)")
with open(out_file, "wb") as f:
    f.write(os_file_content)
