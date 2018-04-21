from re import split


def str2latex(string_list):

    string_list_new =[]
    for string_field in string_list:
         field_split = split("_", string_field)
         num_underscores = len(field_split) - 1
         print(field_split)
         string_field_new = ""
         idx = 0
         for i in field_split:
             if(i is not ''):
                 string_field_new += i
                 if len(field_split) > 1 and idx < num_underscores:
                     string_field_new += "\_"
             idx += 1

         string_list_new.append(string_field_new)

    print(string_list_new)


l = ["lol_ol", "titi", "loiu_ukd_"]
print(l)
str2latex(l)