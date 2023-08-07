import math
import numpy as np
from PIL import Image, ImageDraw

import RoMath
import ParseLn

import g_stamp

g_const_Stamp_baseDir="dat"
g_const_Stamp_stampBaseDir="Stamps"  #base dir dat/Stamps contains the masterkey the keys.txt and all the generated stamp images 
g_const_Stamp_stampBakBaseDir="Bak" # dat/Stamps/Bak directory that contains general background
g_const_Stamp_keyFile="Keys"
g_const_Stamp_masterFile="masterKey"
g_const_Stamp_keySufix=".txt"
g_const_Stamp_imgFile="imgf"
g_const_Stamp_imgFileSufix=".tga"



class gaussianInt:
    m_sigma=1.0
    m_x0=0.0
    m_max=0.0
    m_N=100
    m_I=[] #half side of CDF normalized somewhat incorrectly so that at the end of the computed range the CDF is 1 this avoids a bump on the end when computing gauss rand point loc
    m_X=[] #used for tracking the points at the end of each bar of CDF

    gauss_mult_fact=0.0 #set initit to 1/sigma*sqrt(2pi)

    def __init__(self,
                 sigma,
                 x0=0.0,
                 max_in_sigmas=3,
                 N=1000):
        self.m_sigma=math.abs(sigma)
        if(self.m_sigma==0):
            return False
        self.m_x0=x0
        self.m_max=max_in_sigmas*sigma
        if(self.m_max<0):
            return False
        self.m_N=N
        # calculate the gaussian half integral
        deltaX=m_max/m_N
        X_mid=0.0
        int_bar=[]
        total_int=0.0
        for i_int in range(m_N):
            X_mid+=deltaX/2.0
            m_X[i_int]=i_int*deltaX+deltaX
            gvalue=gauss(X_mid+m_x0)
            int_cur_bar=gvalue*deltaX
            int_bar.append(int_cur_bar)
            total_int+=int_cur_bar
        #find the bars for the normalized integral
        for i_int in range(m_N):
            int_bar[i_int]/=total_int
        #calculate the cumulative distribution function
        I_CDF=0.0
        for i_int in range(m_N):
            I_CDF+=int_bar[i_int]
            m_I.append(I_CDF)
        gauss_mult_fact=1.0/sigma*math.sqrt(2*math.pi)
        return True

    def gauss(x):
        expfac= np.exp(-(x-m_x0)**2/(2* sigma**2))
        return gauss_mult_fact*expfac

    def randPt(seed=1246): #actually semi-random since is renormalized so that it stayes in the computed gaussian
        rand01=random()#random float in range 0.0 to 1.0
        signpos=False
        integratedProb=rand01-0.5
        if rand01 >= 0.0:
            signpos=True
            integratedProb*=2.0
        else:
            integratedProb*=-2.0
        max_i_index=m_N-1
        fixGaussFac=m_I[max_i_index]
        if(fixGaussFac<0.0):
            return 0.0
        retPt=0.0
        for i_int in range(m_N):
            fixedGausInt=m_I(i_int)/fixGaussFac
            if(fixedGaussInt>=integratedProb):
                retPt=m_X[i_int]
                break
        if not signpos:
            retPt*=-1
        retPt+=m_x0
        return retPt

    def randAng(seed=54234):
        rand01=random()
        return 2.0*math.pi*rand01

    def rand2D(center=[0.0, 0.0],seed=9278):
        dr=randPt(seed)
        ang=randAng()
        X=[dr*math.sin(ang), dr*math.cos(ang)]
        offset=X+center
        return offset

    def randJitterAng(center_ang, seed=9472):
        randGauss=randPt(seed)
        new_ang=center_ang + randGauss

#class takes the stamps that were prepared and generates addition smudged signal ready to feed into each NNet train
class StampsNNetFeed:
    m_baseDir=""
    m_dirPre_NNet=""
    m_fileSufix_NNetSig=""
    m_fileSufix_NNetBak=""
    m_dirSigPre=""
    m_dirBakPre=""
    m_smudge_mult_factor_for_offset=0.0 #multiples scale_r to get the smudge sigma
    m_scale_r=0.0
    m_smudge_sigma=0.0
    m_gauss_computed_width_in_sigma=0.0#width out to which gaussian is computed
    m_smudge_ang_sigma=0.0
    m_N_bak_smudge=0

    #working data saved and overwritten for each NNet
    m_NNet_num=0
    m_sigKeys_rptr=[] #signal keys pointing to the raw imgs Stamps/ appropriately smudged
    m_bakKeys_rptr=[] #back keys pointing to the raw imgs Stamps/Bak/ with sig overlaps removed (could be smudged later)
    m_sigKeys=[]
    m_bakKeys=[]

    def __init__(self,
                 baseDir="Stamps/L0",
                 dirPrefixNNet="NNet", # dir for each stamp that will train a nnet dat/Stamps/NNet# 
                 fileSufixNNetSig="_sig", #Keys_sig in dat/Stamps/NNet#/ point to files in dat/Stamps/ useful if multiple files here are signal
                 fileSufixNNetBak="_bak", #Keys_bak in dat/Stamps/NNet#/ points to files in dat/Stamps/Bak
                 dirPrefixSig="Sig",#dir that contains the smeared signal images dat/Stamps/NNet#/Sig masterKey.txt and Keys.txt files here point to img in this dir
                 dirPrefixBak="Bak", #dir contains the masterKey and Keys files that point to images in dat/Stamps/Bak
                 smudge_mult_factor_for_offset=2.0,#multiplies the scale_r
                 gauss_width_in_sigma=3.0,
                 smudge_angle_2pi_divs=48.0,#divides the entire 360 degrees down to a width that is used in smudging the angle
                 bak_smudge_N=3
                 ):
        m_baseDir=baseDir
        m_dirPre_NNet=dirPrefixNNet
        m_fileSufix_NNetSig=fileSufixNNetSig
        m_fileSufix_NNetBak=fileSufixNNetBak
        m_dirSigPre=dirPrefixSig
        m_dirBakPre=dirPrefixBak
        m_smudge_mult_factor_for_offset=smudge_mult_factor_for_offset
        m_gauss_computed_width_in_sigma=gauss_width_in_sigma
        m_smudge_ang_sigma=2.0*math.pi/smudge_angle_2pi_divs
        m_N_bak_smudge=bak_smudge_N
        m_NNet_num=0

    def run():
        #all files in the original keys are linked to stamps
        nnetKeysFilePath=m_baseDir+"/"+g_stamp.keyFile+g_stamp.keySufix
        arr_NNetKeys=ParseLn.readLines(keysFilePath)
        if(len(arr_NNetKeys)<1):
            return False
        for s_NNetKey in arr_NNetKeys:
            clearNNet()
            m_NNet_num=s_NNetKey[0]
            genNNetTrainDat()
            genNNetTrainImgs()
        return True

    def readInMasterKeyArray(masterFileName):
        masterFile_array=ParseLn.readLines(masterFileName)
        s_master=masterFile_array[0]
        return s_master

    def getNumSmudgesPerSig(N_sig, N_bak):
        if(N_sig>=N_bak):
            return 1
        num_smudges_raw=m_N_bak_smudge*N_bak/N_sig
        num_smudges=math.floor(num_smudges_raw)
        return num_smudges

    def imgsLargeEnough(imgDim):
        largest_smudge=m_gauss_computed_width_in_sigma*m_smudge_sigma
        if(largest_smudge>imgDim):
            return False
        return True

    def addSigSmudgesForKey_To_Key(numSmudges, sig_key_arr):
        if(sig_key_arr[2]<0.5):
            return False
        gausInt = gaussianInt(m_smudge_sigma,0.0,m_gauss_computed_width_in_sigma)
        gausInt_ang=gaussianInt(m_smudge_ang_sigma,0.0,m_gauss_computed_width_in_sigma)
        for i in range(numSmudges):
            key_arr_line=[0.0]*len(sig_key_arr)
            for i_key in range(sig_key_arr):
                key_arr_line[i_key]=sig_key_arr[i_key]
            rand_offset=gausInt.rand2D()
            rand_smudge_ang_offset=gausInt_ang.randPt()
            key_arr_line[3]=rand_smudge_ang_offset
            key_arr_line[4]=rand_offset[0]
            key_arr_line[5]=rand_offset[1]
            m_sigKeys_rptr.append(key_arr_line)
        return True

    def addBakSmudgesForKey_ToKey(numSmudges, bak_key_arr):
        if(bak_key_arr[1]>0.5):
            return False
        gausInt=gaussianInt(m_smudge_sigma,0.0, m_gauss_computed_width_in_sigma)
        gausInt_ang=gaussianInt(m_smudge_ang_sigma, 0.0, m_gauss_computed_width_in_sigma)
        for i in range(numSmudges):
            key_arr_line=[0.0]*len(bak_key_arr)
            for i_key in range(bak_key_arr):
                key_arr_line[i_key]=bak_key_arr[i_key]
            rand_offset=gausInt.rand2D()
            rand_smudge_ang_offset=gausInt_ang.randPt()
            key_arr_line[3]=rand_smudge_ang_offset
            key_arr_line[4]=rand_offset[0]
            key_arr_line[5]=rand_offset[1]
            m_bakKeys_rptr.append(key_arr_line)

    def clearNNet():
        m_NNet_num=0
        m_sigKeys_rptr.clear()
        m_bakKeys_rptr.clear()

    def genNNetTrainDat():
        NNetBaseDir=m_baseDir + "/" +  m_dirPre_NNet + ParseLn.numberToString(m_NNet_num)+"/"
        masterKeyPath=NNetBaseDir+g_stamp.masterFile+g_stamp.keySufix
        signalKeysPath=NNetBaseDir+g_stamp.keyFile+m_fileSufix_NNetSig+g_stamp.keySufix
        bakKeysPath=NNetBaseDir+g_stamp.keyFile+m_fileSufix_NNetBak + g_stamp.keySufix
        if(len(s_masterArr)<1):
            return False
        s_masterArr=readInMasterKeyArray(masterKeyPath)
        image_largest_dim=s_masterArr[1]
        if not imgsLargeEnough(image_largest_dim):
            return False
        m_scale_r=s_masterArr[0]
        m_smudge_sigma=m_smudge_mult_factor_for_offset*m_scale_r
        N_raw_sig=s_masterArr[4] #number of image files in the Stamps dir
        N_raw_bak=s_masterArr[5] #number of image files in the Stamps/Bak dir
        if(N_raw_sig <1 or N_raw_bak<1):
            return False
        raw_sig_keys_arr=ParseLn.readLines(signalKeysPath)
        raw_bak_keys_arr=ParseLn.readLines(bakKeysPath)
        if(len(raw_sig_keys_arr)<1 or len(raw_bak_keys_arr)<1 ):
            return False
        #each of the signals needed to be smudged to increase the number of signal bringing it up to the number of background
        num_sig_smudge=getNumSmudgesPerSig(N_raw_sig, N_raw_bak)
        for i_sig in range(len(raw_sig_keys_arr)):
            raw_sig_key_line = raw_sig_keys_arr[i]
            if(raw_sig_key_line[2]>0.0):
                addSigSmudgesForKey_To_Key(num_sig_smudge, raw_sig_key_line)
        extra_smudges = N_raw_bak*m_N_bak_smudge- len(m_sigKeys_rptr)
        if(extra_smudges>=1):
            addSigSmudgesForKey_To_Key(extra_smudges,raw_sig_keys_arr[0])
        #now smudge bak
        tot_N_bak=N_raw_bak*m_N_bak_smudge
        for i_bak in range(len(raw_bak_keys_arr)):
            raw_bak_key_line = raw_bak_keys_arr[i]
            if(raw_bak_key_line[2]<=0.0):
                addBakSmudgesForKey_ToKey(m_N_bak_smudge, raw_bak_key_line)
        return True

    #must be run after genNNetTrain Dat generates the appropriate images according to the the keys and generates the key files with refs to the images
    def genNNetTrainImgs():
        NNetDir=m_dirPre_NNet+ParseLn.numberToString(m_NNet_num)+"/"
        sigRawImgFileDir=m_baseDir + "/"
        bakRawImgFileDir=m_baseDir + "/" + m_dirBakPre + "/"
        N_sig=len(m_sigKeys_rptr)
        if(N_sig<1):
            return False
        for i_key in range(N_sig):
            img_key_line=[]
            raw_key_line=m_sigKeys_rptr[i_key]
            raw_line_N=len(raw_key_line)
            for i_line in range(raw_line_N):
                img_key_line.append(raw_key_line[i_line])
            img_key_line[1]= RoMath.angTo2Pi(raw_key_line[1]+raw_key_line[3])
            imgFilePath = sigRawImgFileDir + g_stamp.imgFile + ParseLn.numberToString(raw_key_line[0]) + g_stamp.imgFileSufix
            imgOutFilePath = sigRawImgFileDir + NNetDir + m_dirSigPre+"/" + g_stamp.imgFile + ParseLn.numberToString(i_key) + g_stamp.imgFileSufix
            img_key_line[0]=i_key
            raw_img=Image.open(imgFilePath)
            ang_rot_degrees = np.rad2deg(raw_key_line[3])
            rot_img=raw_img.rotate(ang_rot_degrees)
            #affine transform 6 tuple (a, b, c, d, e, f) (x,y) goes to (ax+by+c, dx+ey+f)
            aff_c=img_key_line[4]
            aff_f=img_key_line[5]
            trans_img=rot_img.transform(rot_img.size, Image.AFFINE, (0, 0,aff_c, 0,0,aff_f), fillcolor=0x00)
            trans_img.save(imgOutFilePath)
            m_sigKeys.append(img_key_line)
        #now to bak
        N_bak=len(m_bakKeys_rptr)
        for i_key in range(N_bak):
            img_key_line=[]
            raw_key_line=m_bakKeys_rptr[i_key]
            raw_line_N=len(raw_key_line)
            for i_line in range(raw_line_N):
                img_key_line.append(raw_key_line[i_line])
            img_key_line[1]=RoMath.angTo2Pi(raw_key_line[1]+raw_key_line[3])
            imgFilePath=bakRawImgFileDir + g_stamp.imgFile + ParseLn.numberToString(raw_key_line[0])+g_stamp.imgFileSufix
            imgOutFilePath = sigRawImgFileDir + NNetDir + m_dirBakPre + "/" + g_stamp.imgFile + ParseLn.numberToString(i_key) + g_stamp.imgFileSufix
            img_key_line[0]=i_key
            raw_img=Image.open(imgFilePath)
            ang_rot_degrees=np.rad2deg(raw_key_line[3])
            rot_img=raw_img.rotate(ang_rot_degrees)
            aff_c=img_key_line[4]
            aff_f=img_key_line[5]
            trans_img=rot_img.transform(rot_img.size, Image.AFFINE, (0, 0, aff_c, 0, 0, aff_f), fillcolor=0x00)
            trans_img.save(imgOutFilePath)
            m_bakKeys.append(img_key_line)

        return True