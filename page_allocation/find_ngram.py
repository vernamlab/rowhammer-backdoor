from matplotlib import pyplot as plt
import matplotlib as mpl
import colorsys

def lighten_color(color, amount=0.5):
    """
    Lightens the given color by multiplying (1-luminosity) by the given amount.
    Input can be matplotlib color string, hex string, or RGB tuple.

    Examples:
    >> lighten_color('g', 0.3)
    >> lighten_color('#F034A3', 0.6)
    >> lighten_color((.3,.55,.1), 0.5)
    """
    import matplotlib.colors as mc
    import colorsys
    try:
        c = mc.cnames[color]
    except:
        c = color
    c = colorsys.rgb_to_hls(*mc.to_rgb(c))
    return colorsys.hls_to_rgb(c[0], 1 - amount * (1 - c[1]), c[2])


import numpy as np

# Python 3 program to find the longest repeated
# non-overlapping substring
 
# Returns the longest repeating non-overlapping
# substring in str
# Original code is taken from 
# https://www.geeksforgeeks.org/longest-repeating-and-non-overlapping-substring/
def longestRepeatedSubstring(strin, out):
    strin = strin.split("\n")
    n = len(strin)
    LCSRe = [[0 for x in range(n + 1)]
                for y in range(n + 1)]
 
    res = [] # To store result
    res_length = 0 # To store length of result
 
    # building table in bottom-up manner
    index = 0
    for i in range(1, n + 1):
        for j in range(i + 1, n + 1):
             
            # (j-i) > LCSRe[i-1][j-1] to remove
            # overlapping
            if (strin[i - 1] == strin[j - 1] and
                LCSRe[i - 1][j - 1] < (j - i)):
                LCSRe[i][j] = LCSRe[i - 1][j - 1] + 1
 
                # updating maximum length of the
                # substring and updating the finishing
                # index of the suffix
                if True: #(LCSRe[i][j] > res_length):
                    res_length = LCSRe[i][j]
                    index = max(i, index)
                    out.write(str(index)+" "+str(res_length)+"\n")
                 
            else:
                LCSRe[i][j] = 0
 
    # If we have non-empty result, then insert
    # all characters from first character to
    # last character of string
    #if (res_length > 0):
    #    for i in range(index - res_length + 1,
    #                                index + 1):
    #        res.append(str[i - 1])
    
    return res
 
# Driver Code
if __name__ == "__main__":
    START=1
    STOP=2
    STEP=1
    fig = plt.figure()
    ax = fig.add_subplot()
    fig.subplots_adjust(top=0.85)
    for idx in range(START,STOP,STEP):
        log1 = str(idx)+"_buffer_addr.txt"
        log2 = str(idx)+"_file_addr.txt"
        log3 = str(idx)+"_unmapped_addr.txt"
        log_out = str(idx)+"_long_repeating_substring.txt"
        with open(log1,"r") as f:
            adr1 = f.read()
        with open(log2,"r") as f:
            adr2 = f.read()
        with open(log3,"r") as f:
            adr3 = f.read()

        #adr_merged = adr1 + "\n" +  adr2
        adr_merged = adr2 + "\n" +  adr3

        #print("mmap1 and mmap2")
        with open(log_out,"w") as out:
            common1 = longestRepeatedSubstring(adr_merged, out)
        
        with open(log_out,"r") as out:
            a = out.readlines()
            x = np.array([int(l.replace("\n","").split(" ")[0]) for l in a])
            y = np.array([int(l.replace("\n","").split(" ")[1]) for l in a])
            print("max y is ",max(y))
            # print(x)
            # print(y)
        plt.plot(x,y, color=lighten_color('r',2*idx/STOP))
        for x_,y_ in zip(x,y):
            if y_>=len(y):
                ax.annotate(str(x_)+","+str(y_), xy=(x_, y_), xytext=(x_+25, y_+5),
                            arrowprops=dict(facecolor='black', shrink=0.05))
    plt.xlabel("Page Number")
    plt.ylabel("Length of Consecutive Blocks")
    plt.savefig("figure.png")
            

        
    #count1 = common1.split("\n")
    #print(len(common1))
    """
    adr_merged = adr2 + " " +  adr3
    print("mmap2 and mmap3")
    common2 = longestRepeatedSubstring(adr_merged)
    count1 = common2.split(" ")
    print(len(count1))

    print("mmap1 and mmap2 and mmap3")

    common_final = longestRepeatedSubstring(common1 + " " + common2)
    count1 = common_final.split(" ")
    print(len(count1))
    """
    # print(common1)
    # print(common2)
    # print(common_final)

        
# This code is contributed by ita_c