#!/usr/bin/python3

import random

total_kv_num   =  100000
unique_key_num =  20000

unique_key_list = []

real_unique_key = set()

random.seed(100)

while(len(unique_key_list)!=unique_key_num):
    temp = random.randint(28495,234878620)
    if temp not in unique_key_list:
        unique_key_list.append(temp)

with open("key_value_demo.txt","w") as f:
    for i in range(total_kv_num):
        temp = random.choices(unique_key_list)[0]
        if temp not in real_unique_key:
            real_unique_key.add(temp)
        f.write(str(temp)+"\n")


print("total kv pair number: "+str(total_kv_num))
print("unique key number: "+str(len(real_unique_key)))