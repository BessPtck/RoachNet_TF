import os
import shutil
import math
import numpy as np
from PIL import Image, ImageDraw

import g_stamp

import ParseLn
import StampsGen
import StampsPreFeed
import StampsNNetFeed

def math_powerXseries(x,n):# 1 + x+x^2 + x^3 +...x^n
    # multiply by x the series S
    # ends up with x*S - S = x^(n+1)-1
    X_N1=x**(n+1)
    X_N1_1=X_N1 - 1.0
    X_1=x-1.0
    if X_1==0.0:
        return 1 + n
    return X_N1_1/X_1

def sizeOfHexEye(scale_r, eyeLevel):
    R=scale_r*2**eyeLevel
    side_extent = math_powerXseries(0.5,eyeLevel)
    mini_side = 0.5**eyeLevel
    side_extent*= R * math.sqrt(3.0)/2.0
    mini_side*=R*0.5
    return math.sqrt(side_extent**2 + mini_side**2)


def genRawStamps(scale_r,
                 smudge_mult_factor_for_offset,
                 gauss_width_in_sigma,
                 smudge_angle_2pi_divs,
                 bak_smudge_N,
                 baseDir,
                 subDir):
    clearPath=baseDir+"/"+subDir
    if os.path.exists(clearPath):
        shutil.rmtree(clearPath)

    #generate and write the stamps
    eyeDim=sizeOfHexEye(scale_r,2)+scale_r #luna will extend one scale_r over the size of the eye
    smudge_shift=smudge_mult_factor_for_offset*scale_r*gauss_width_in_sigma
    smudge_padding = smudge_shift+scale_r
    fullImgDim=math.ceil(eyeDim+smudge_padding)
    
    raw_stamps_gen = StampsGen.StampGen(fullImgDim,scale_r)
    raw_keys=[]
    raw_imgs=raw_stamps_gen.run(raw_keys)
    print("Number of stamps generated: ", len(raw_keys))

    raw_master=[[0]*g_stamp.masterKey_len]
    raw_master[0][0]=scale_r
    raw_master[0][1]=eyeDim
    raw_master[0][2]=fullImgDim
    N_stamps = len(raw_keys)
    raw_master[0][3]=2*N_stamps
    raw_master[0][4]=N_stamps
    raw_master[0][5]=N_stamps
    raw_master[0][6]=smudge_angle_2pi_divs
    raw_master[0][7]=smudge_mult_factor_for_offset
    raw_master[0][8]=bak_smudge_N
    raw_master[0][9]=0
    raw_master[0][10]=gauss_width_in_sigma

    outDir = baseDir+"/"+subDir
    os.mkdir(outDir)
    if not os.path.exists(outDir):
        print("Error cant create: ", outDir)
        return False
    print("Writing to Directory: ", outDir)

    if N_stamps != len(raw_imgs):
        print("error array len do not match")
        return False
    print("<",end="")
    for i_f in range(N_stamps):
        s_key=raw_keys[i_f]
        key_id=s_key[0]
        outFile=outDir+"/"+g_stamp.imgFile+ParseLn.numberToString(key_id)+g_stamp.imgFileSufix
        raw_imgs[i_f].save(outFile)
        if i_f%10 == 0:
            print("*",end="")
    print(">",end="")
    keysOutFile=outDir+"/"+g_stamp.keyFile+g_stamp.keySufix
    masterOutFile=outDir+"/"+g_stamp.masterFile+g_stamp.keySufix
    ParseLn.writeLineS(keysOutFile,raw_keys)
    ParseLn.writeLineS(masterOutFile,raw_master)
    return True
        

def GenStamps(scale_r=7.0,
              smudge_mult_factor_for_offset=2.0,#determines how much signal and background are smudged in translation, this is given in terms of r
              gauss_width_in_sigma=3.0, #how far out the smudge will go in sigmal
              smudge_angle_2pi_divs=48.0,
              bak_smudge_N=3,
              baseDir="Dat",
              subDir="L0",
              sigDir="Sig",
              bakDir="Bak",
              nnetDir="NNet",
              sigSufix="_sig",
              bakSufix="_bak"
              ):
    genRawStamps(scale_r, smudge_mult_factor_for_offset, gauss_width_in_sigma, smudge_angle_2pi_divs, bak_smudge_N, baseDir, subDir)

    #take the stamps create the background in the background dir and create the key files that exclude overlaps in the NNet## directories
    preFeed=StampsPreFeed.StampPreFeed(math.pi/12.0,
                                       0.2,
                                       outDir,
                                       sigDir,
                                       bakDir,
                                       nnetDir,
                                       sigSufix,
                                       bakSufix)
    preFeed.run()

    #now do the smearing generating both signal and background smeared images and the corresponding keys
    NNetFeed = StampsNNetFeed.StampsNNetFeed(outDir,
                                             nnetDir,
                                             sigSufix,
                                             bakSufix,
                                             sigDir,
                                             bakDir,
                                             smudge_mult_factor_for_offset,
                                             gauss_width_in_sigma,
                                             smudge_angle_2pi_divs,
                                             bak_smudge_N)
    NNetFeed.run()
    return True


