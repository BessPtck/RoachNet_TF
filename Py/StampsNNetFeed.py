import os
import math
import numpy as np
import random
from PIL import Image, ImageDraw

import RoMath
import ParseLn

import g_stamp

#constants
rand_seed_0=571942
rand_seed_1=9563274
rand_seed_2=2493458
rand_seed_3=760385

class gaussianInt:
    def __init__(self,
                 sigma,
                 x0=0.0,
                 max_in_sigmas=3,
                 N=1000):
        self.sigma=abs(sigma)
        if(self.sigma==0):
            return
        self.x0=x0
        self.max=max_in_sigmas*sigma
        if(self.max<0):
            return
        self.N=N
        self.gauss_mult_fact=1.0/sigma*math.sqrt(2*math.pi)#set to 1/sigma*sqrt(2pi)
        self.I=[] #half side of CDF normalized somewhat incorrectly so that at the end of the computed range the CDF is 1 this avoids a bump on the end when computing gauss rand point loc
        self.I2D=[] #integral setup such that as rings get larger the prob is scaled up roughly by the ring area
        self.X=[] #used for tracking the points at the end of each bar of CDF
        # calculate the gaussian half integral
        self.deltaX=self.max/self.N
        X_mid=0.0
        total_int=0.0
        total_int2D=0.0 #ring gauss
        unitArea=self.deltaX**2 #ring gauss, ignore the 2pi since it cancels out
        X_lo=0.0 #ring gauss
        deltaHalf=self.deltaX/2.0
        #calculate the cumulative distribution function, this is in coords one high for X
        for i_int in range(self.N):
            X_hi=i_int*self.deltaX+self.deltaX
            self.X.append(X_hi)
            X_mid=X_hi-deltaHalf
            gvalue=self.gauss(X_mid+self.x0)
            total_int+=gvalue #really gvalue*deltaX but this is going to be normalized anyway
            self.I.append(total_int)
            #scale the 2D gaussian by its ring area
            ringArea=(X_hi**2-X_lo**2) #ignore the 2pi again
            normFac=ringArea/unitArea
            gvalue2D=gvalue*normFac
            total_int2D+=gvalue2D
            self.I2D.append(total_int2D)
            X_lo=X_hi
        #normalize the cumulative distribution function
        for i_int in range(self.N):
            self.I[i_int]/=total_int
            self.I2D[i_int]/=total_int2D
        return 

    def gauss(self,x):
        expfac= np.exp(-(x-self.x0)**2/(2* self.sigma**2))
        return self.gauss_mult_fact*expfac

    def randPt(self,seed=1246): #actually semi-random since is renormalized so that it stayes in the computed gaussian
        rand01=random.random()#random float in range 0.0 to 1.0
        signpos=False
        integratedProb=rand01-0.5 #random number from -1/2 to +1/2
        integratedProb*=2.0 #random float number from -1 to +1
        if  integratedProb >= 0.0:
            signpos=True
        else:
            integratedProb=abs(integratedProb)
        retPt=0.0
        for i_int in range(self.N):
            if(self.I[i_int]>integratedProb):
                retPt=self.X[i_int]
                break
        if not signpos:
            retPt*=-1
        retPt+=self.x0
        return retPt

    def randPt2D(self,seed=35024): #actually semi-random since is renormalized so that it stayes in the computed gaussian
        rand01=random.random()#random float in range 0.0 to 1.0
        signpos=False
        integratedProb=rand01-0.5 #random number from -1/2 to +1/2
        integratedProb*=2.0 #random float number from -1 to +1
        if  integratedProb >= 0.0:
            signpos=True
        else:
            integratedProb=abs(integratedProb)
        retPt=0.0
        for i_int in range(self.N):
            if(self.I2D[i_int]>integratedProb):
                retPt=self.X[i_int]
                break
        if not signpos:
            retPt*=-1
        retPt+=self.x0
        return retPt

    def randAng(self,seed=54234):
        rand01=random.random()
        return 2.0*math.pi*rand01

    def rand2D(self,center=[0.0, 0.0],seed=9278):
        dr=self.randPt2D(seed)
        ang=self.randAng()
        X=[dr*math.sin(ang), dr*math.cos(ang)]
        offset=[0.0,0.0]
        for i in range(2):
            offset[i]=X[i]+center[i]
        return offset
        

#class takes the stamps that were prepared and generates addition smudged signal ready to feed into each NNet train
class StampsNNetFeed:
    
    def __init__(self,
                 baseDir="Stamps/L0",
                 dirPrefixNNet="NNet", # dir for each stamp that will train a nnet dat/Stamps/NNet# 
                 fileSufixNNetSig="_sig", #Keys_sig in dat/Stamps/NNet#/ point to files in dat/Stamps/ useful if multiple files here are signal
                 fileSufixNNetBak="_bak", #Keys_bak in dat/Stamps/NNet#/ points to files in dat/Stamps/Bak
                 dirPrefixSig="Sig",#dir that contains the smeared signal images dat/Stamps/NNet#/Sig masterKey.txt and Keys.txt files here point to img in this dir
                 dirPrefixBak="Bak", #dir contains the masterKey and Keys files that point to images in dat/Stamps/Bak
                 fileSmudgeKeyLabel="_smudge",
                 smudge_mult_factor_for_offset=0.5,#multiplies the scale_r
                 gauss_width_in_sigma=3.0,
                 smudge_angle_2pi_divs=120.0,#divides the entire 360 degrees down to a width that is used in smudging the angle
                 bak_smudge_N=3
                 ):
        self.baseDir=baseDir
        self.dirPre_NNet=dirPrefixNNet
        self.fileSufix_NNetSig=fileSufixNNetSig
        self.fileSufix_NNetBak=fileSufixNNetBak
        self.dirSigPre=dirPrefixSig
        self.dirBakPre=dirPrefixBak
        self.fileSmudgeKeyLabel=fileSmudgeKeyLabel
        self.smudge_mult_factor_for_offset=smudge_mult_factor_for_offset
        self.gauss_computed_width_in_sigma=gauss_width_in_sigma
        self.smudge_ang_sigma=2.0*math.pi/smudge_angle_2pi_divs
        self.N_bak_smudge=bak_smudge_N
        #working data saved and overwritten for each NNet
        self.scale_r=0.0
        self.smudge_sigma=0.0
        self.NNet_num=0
        self.sigKeys_rptr=[] #signal keys pointing to the raw imgs Stamps/ appropriately smudged
        self.bakKeys_rptr=[] #back keys pointing to the raw imgs Stamps/Bak/ with sig overlaps removed (could be smudged later)
        self.sigKeys=[]
        self.bakKeys=[]
        
    def run(self):
        #all files in the original keys are linked to stamps
        nnetKeysFilePath=self.baseDir+"/"+g_stamp.keyFile+g_stamp.keySufix
        arr_NNetKeys=ParseLn.readLines(nnetKeysFilePath)
        if(len(arr_NNetKeys)<1):
            return False
        for s_NNetKey in arr_NNetKeys:
            self.clearNNet()
            self.NNet_num=int(s_NNetKey[0])
            if self.genNNetTrainDat():
                self.genNNetTrainImgs() 
        return True

    def readInMasterKeyArray(self, masterFileName):
        masterFile_array=ParseLn.readLines(masterFileName)
        s_master=masterFile_array[0]
        return s_master

    def getNumSmudgesPerSig(self,N_sig, N_bak):
        if(N_sig>=N_bak or N_bak<=0):
            return 1
        num_smudges_raw=self.N_bak_smudge*N_bak/N_sig
        num_smudges=int(math.floor(num_smudges_raw))
        return num_smudges

    def imgsLargeEnough(self,imgDim):
        largest_smudge=self.gauss_computed_width_in_sigma*self.smudge_sigma
        if(largest_smudge>imgDim):
            return False
        return True

    def addSigSmudgesForKey_To_Key(self, numSmudges, sig_key_arr):
        rand_seed=int(sig_key_arr[0]+rand_seed_0)
        if(sig_key_arr[2]<0.5):
            return False
        gausInt = gaussianInt(self.smudge_sigma,0.0,self.gauss_computed_width_in_sigma)
        gausInt_ang=gaussianInt(self.smudge_ang_sigma,0.0,self.gauss_computed_width_in_sigma)
        for i_smudge in range(numSmudges):
            key_arr_line=[0.0]*len(sig_key_arr)
            for i_key in range(len(sig_key_arr)):
                key_arr_line[i_key]=sig_key_arr[i_key]
            rand_offset=gausInt.rand2D([0.0, 0.0],rand_seed*i_smudge)
            rand_smudge_ang_offset=RoMath.angTo2Pi(gausInt_ang.randPt(rand_seed*i_smudge+rand_seed_1))
            key_arr_line[3]=rand_smudge_ang_offset
            key_arr_line[4]=rand_offset[0]
            key_arr_line[5]=rand_offset[1]
            self.sigKeys_rptr.append(key_arr_line)
        return True

    def addBakSmudgesForKey_ToKey(self,numSmudges, bak_key_arr):
        rand_seed=int(bak_key_arr[0]+rand_seed_2)
        if(bak_key_arr[2]>0.5):
            return False
        gausInt=gaussianInt(self.smudge_sigma,0.0, self.gauss_computed_width_in_sigma)
        gausInt_ang=gaussianInt(self.smudge_ang_sigma, 0.0, self.gauss_computed_width_in_sigma)
        for i_smudge in range(numSmudges):
            key_arr_line=[0.0]*len(bak_key_arr)
            for i_key in range(len(bak_key_arr)):
                key_arr_line[i_key]=bak_key_arr[i_key]
            rand_offset=gausInt.rand2D([0.0,0.0],rand_seed*i_smudge)
            rand_smudge_ang_offset=RoMath.angTo2Pi(gausInt_ang.randPt(rand_seed*i_smudge+rand_seed_3))
            key_arr_line[3]=rand_smudge_ang_offset
            key_arr_line[4]=rand_offset[0]
            key_arr_line[5]=rand_offset[1]
            self.bakKeys_rptr.append(key_arr_line)
        return True
    
    def writeKeys(self):
        NNetBaseDir=self.baseDir + "/" +  self.dirPre_NNet + ParseLn.numberToString(self.NNet_num)+"/"
        masterKeyPathIn=NNetBaseDir+g_stamp.masterFile+g_stamp.keySufix
        s_masterArr=self.readInMasterKeyArray(masterKeyPathIn)
        if(len(s_masterArr)<1):
            return 
        orig_N_sig=s_masterArr[4]

        masterKeyPath=NNetBaseDir+g_stamp.masterFile+self.fileSmudgeKeyLabel+g_stamp.keySufix
        sigKeysPath=NNetBaseDir+g_stamp.keyFile+self.fileSmudgeKeyLabel+self.fileSufix_NNetSig + g_stamp.keySufix
        bakKeysPath=NNetBaseDir+g_stamp.keyFile+self.fileSmudgeKeyLabel+self.fileSufix_NNetBak + g_stamp.keySufix
        N_sig=len(self.sigKeys)
        N_bak=len(self.bakKeys)
        N_tot=N_sig+N_bak
        s_masterArr[3]=N_tot
        s_masterArr[4]=N_sig
        s_masterArr[5]=N_bak
        s_masterArr[8]=self.N_bak_smudge
        s_masterArr[9]=self.getNumSmudgesPerSig(orig_N_sig,N_bak)
        ParseLn.writeLine(masterKeyPath,s_masterArr)
        ParseLn.writeLineS(sigKeysPath,self.sigKeys)
        ParseLn.writeLineS(bakKeysPath,self.bakKeys)
        
    def clearNNet(self):
        self.scale_r=0.0
        self.smudge_sigma=0.0
        self.NNet_num=0
        self.sigKeys_rptr.clear()
        self.bakKeys_rptr.clear()
        self.sigKeys.clear()
        self.bakKeys.clear()

    def genNNetTrainDat(self):
        NNetBaseDir=self.baseDir + "/" +  self.dirPre_NNet + ParseLn.numberToString(self.NNet_num)+"/"
        IsNNetToTrain = os.path.exists(NNetBaseDir)
        if not IsNNetToTrain:
            return False
        masterKeyPath=NNetBaseDir+g_stamp.masterFile+g_stamp.keySufix
        signalKeysPath=NNetBaseDir+g_stamp.keyFile+self.fileSufix_NNetSig+g_stamp.keySufix
        bakKeysPath=NNetBaseDir+g_stamp.keyFile+self.fileSufix_NNetBak + g_stamp.keySufix
        s_masterArr=self.readInMasterKeyArray(masterKeyPath)
        if(len(s_masterArr)<1):
            return False
        self.scale_r=s_masterArr[0]
        self.smudge_sigma=self.smudge_mult_factor_for_offset*self.scale_r
        image_largest_dim=s_masterArr[2]
        if not self.imgsLargeEnough(image_largest_dim):
            return False
        N_raw_sig=s_masterArr[4] #number of image files in the Stamps dir
        N_raw_bak=s_masterArr[5] #number of image files in the Stamps/Bak dir
        if(N_raw_sig <1 or N_raw_bak<1):
            return False
        raw_sig_keys_arr=ParseLn.readLines(signalKeysPath)
        raw_bak_keys_arr=ParseLn.readLines(bakKeysPath)
        if(len(raw_sig_keys_arr)<1 or len(raw_bak_keys_arr)<1 ):
            return False
        #each of the signals needed to be smudged to increase the number of signal bringing it up to the number of background
        num_sig_smudge=self.getNumSmudgesPerSig(N_raw_sig, N_raw_bak)
        print("Gen sig smudges")
        print("<", end="")
        for i_sig in range(len(raw_sig_keys_arr)):
            raw_sig_key_line = raw_sig_keys_arr[i_sig]
            if(raw_sig_key_line[2]>0.0):
                self.addSigSmudgesForKey_To_Key(num_sig_smudge, raw_sig_key_line)
                print("*", end="")
        extra_smudges = N_raw_bak*self.N_bak_smudge- len(self.sigKeys_rptr)
        if(extra_smudges>=1):
            self.addSigSmudgesForKey_To_Key(extra_smudges,raw_sig_keys_arr[0])
            print("*",end="")
        print(">")
        #now smudge bak
        print("Gen Bak smudges")
        print("<",end="")
        tot_N_bak=N_raw_bak*self.N_bak_smudge
        for i_bak in range(len(raw_bak_keys_arr)):
            raw_bak_key_line = raw_bak_keys_arr[i_bak]
            if(raw_bak_key_line[2]<=0.0):
                self.addBakSmudgesForKey_ToKey(self.N_bak_smudge, raw_bak_key_line)
                print("*",end="")
        print(">")
        return True

    #must be run after genNNetTrain Dat generates the appropriate images according to the the keys and generates the key files with refs to the images
    def genNNetTrainImgs(self):
        NNetDir=self.dirPre_NNet+ParseLn.numberToString(self.NNet_num)+"/"
        sigRawImgFileDir=self.baseDir + "/"
        bakRawImgFileDir=self.baseDir + "/" + self.dirBakPre + "/"
        N_sig=len(self.sigKeys_rptr)
        print("Gen Imgs for net: ", NNetDir)
        print("<",end="")
        if(N_sig<1):
            return False
        for i_key in range(N_sig):
            img_key_line=[]
            raw_key_line=self.sigKeys_rptr[i_key]
            raw_line_N=len(raw_key_line)
            for i_line in range(raw_line_N):
                img_key_line.append(raw_key_line[i_line])
            imgFilePath = sigRawImgFileDir + g_stamp.imgFile + ParseLn.numberToString(raw_key_line[0]) + g_stamp.imgFileSufix
            imgOutFilePath = sigRawImgFileDir + NNetDir + self.dirSigPre+"/" + g_stamp.imgFile + ParseLn.numberToString(i_key) + g_stamp.imgFileSufix
            img_key_line[0]=i_key
            raw_img=Image.open(imgFilePath)
            ang_rot_degrees = np.rad2deg(raw_key_line[3])
            rot_img=raw_img.rotate(ang_rot_degrees)
            #affine transform 6 tuple (a, b, c, d, e, f) (x,y) goes to (ax+by+c, dx+ey+f)
            aff_c=int(img_key_line[4])
            aff_f=int(img_key_line[5])
            trans_img=rot_img.transform(rot_img.size, Image.AFFINE, (1, 0,aff_c, 0,1,aff_f), fillcolor=0x00)
            trans_img.save(imgOutFilePath)  
            self.sigKeys.append(img_key_line)
            print("*", end="")
        #now to bak
        N_bak=len(self.bakKeys_rptr)
        for i_key in range(N_bak):
            img_key_line=[]
            raw_key_line=self.bakKeys_rptr[i_key]
            raw_line_N=len(raw_key_line)
            for i_line in range(raw_line_N):
                img_key_line.append(raw_key_line[i_line])
            imgFilePath=bakRawImgFileDir + g_stamp.imgFile + ParseLn.numberToString(raw_key_line[0])+g_stamp.imgFileSufix
            imgOutFilePath = sigRawImgFileDir + NNetDir + self.dirBakPre + "/" + g_stamp.imgFile + ParseLn.numberToString(i_key) + g_stamp.imgFileSufix
            img_key_line[0]=i_key
            raw_img=Image.open(imgFilePath)
            ang_rot_degrees=np.rad2deg(raw_key_line[3])
            rot_img=raw_img.rotate(ang_rot_degrees)
            aff_c=img_key_line[4]
            aff_f=img_key_line[5]
            trans_img=rot_img.transform(rot_img.size, Image.AFFINE, (1, 0, aff_c, 0, 1, aff_f), fillcolor=0x00)
            trans_img.save(imgOutFilePath) 
            self.bakKeys.append(img_key_line)
            print("*", end="")
        print(">")
        self.writeKeys()
        return True