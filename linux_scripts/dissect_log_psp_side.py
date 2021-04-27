#!/usr/bin/env python3

import re

regex_initial = re.compile(".*WirelessPSPPluginDevice.cpp:\d*:Sent:.*")
regex_initial_recv = re.compile(".*WirelessPSPPluginDevice.cpp:\d*:Received:.*")
regex_hexdump = re.compile("\d\d\d\d\d\d .*")
dump_file = ""
match_next = False

with open("log.txt", "r") as input_file:
    for line in input_file:
        if not match_next:
            result = regex_initial.match(line)
            if result != None:
                match_next = True
            else:
                result = regex_initial_recv.match(line)
                if result != None:
                    match_next = True
        else:
            result = regex_hexdump.match(line)
            if result != None:
                dump_file += line
            else:
                match_next = False


text_file = open("pspside.txt", "w")
text_file.write(dump_file)
text_file.close()
