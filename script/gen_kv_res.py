#!/usr/bin/python3

kv_store = set()

unique_key_num = 0
    
with open("key_value.txt","r") as input_kv:
    with open("key_value_res.txt","w") as res:
        temp = input_kv.readlines()
        # for each key, first lookup
        for i in temp:
            # if find key in the set, print the key
            if i in kv_store:
                res.write(i)
            #else, print not find and insert it into set
            else:
                res.write("not find\n")
                kv_store.add(i)
                unique_key_num+=1
print("unique key num: "+str(unique_key_num))
