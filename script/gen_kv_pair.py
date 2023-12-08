#!/usr/bin/python3

import random

total_kv_num   =  100000
unique_key_num =  20000

unique_key_list = set()

key_store = set()

random.seed(100)

while(len(unique_key_list)!=unique_key_num):
    temp = random.randint(28495,234878620)
    if temp not in unique_key_list:
        unique_key_list.add(temp)

unique_key_list = list(unique_key_list)

file_n = "test_case/key_value_100K"
with open(file_n+".txt","w") as output_kv:
    with open(file_n+"_res.txt","w") as output_res:
        # iterate for total_kv_num times
        for i in range(total_kv_num):
            # ramdonly pick a key
            temp = random.choices(unique_key_list)[0]
            # write the key out to file
            output_kv.write(str(temp)+"\n")
            # if the key does not exist in the key_store, insert it, output not find
            if temp not in key_store:
                key_store.add(temp)
                output_res.write("not find\n")
            # else, output the key
            else:
                output_res.write(str(temp)+"\n")

with open(file_n + "_key.txt","w") as f:
    temp = list(key_store)
    for i in temp:
        f.write(str(i)+"\n")

print("total kv pair number: "+str(total_kv_num))
print("unique key number: "+str(len(key_store)))