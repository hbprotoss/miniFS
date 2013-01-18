import os

os.system("chdir")
while(True):
    block = int(input("block:"), base = 16)
    size = int(input("size:"))
    os.system("hexdump /skip:%d /keep:%d test" % (block * 4 * 1024, size))
    input("press any key to continue")
    os.system("cls")
