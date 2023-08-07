

def numberToString(num):
    retStr=""
    if num<10:
        retStr="00" + str(num)
    elif num<100:
        retStr="0"+str(num)
    else:
        retStr=str(num)
    return retStr

def readLines(ffileIn):
    lines_arr=[]
    line_length=0
    ffile=open(ffileIn, "r")
    fline=ffile.readline()
    while fline:
        fline=ffile.readline()
        commaloc=fline.find(",")
        retline_ar=[]
        while commaloc>=0:
            
            fieldstr=fline[:commaloc]
            retline_ar.append(float(fieldstr))
            line_length+=1
            commaloc+=1
            remstr=fline[commaloc:]
            fline=remstr
            commaloc=fline.find(",")
        lines_arr.append(retline_ar)

    ffile.close()
    return lines_arr

def lineToStr(line_ar):
    outStr=""
    for i in range(len(line_ar)):
        outStr+=str(line_ar[i])
        outStr+=","
    outStr+="\n"
    return outStr

def writeLine(ffileOut,line):
    ofile=open(ffileOut,"w")
    outStr=lineToStr(line)
    ofile.write(outStr)
    ofile.close()

def writeLineS(ffileOut, lineArr):
    ofile=open(ffileOut,"w")
    len_multi_line_array=len(lineArr)
    for i in range(len_multi_line_array):
        line_ar=lineArr[i]
        outStr=lineToStr(line_ar)
        ofile.write(outStr)
    ofile.close()