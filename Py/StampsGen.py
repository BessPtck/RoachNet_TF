import g_stamp
import math
import RoMath
import numpy as np
from PIL import Image, ImageDraw, ImageOps

#effectively constants
m_stampZeroIntensity=0.0
m_stamp_max_ang_rad=3.1




class StampGen:
    #########
    #stampsKey variables
    #s_stampsKey=[r,#0 scale of small r
    #             D,#1 dim of the full size of the stamp 
    #             maxDim, #2 max size of image, used when image is slid by offset to make sure image is still large enough to account for slide
    #             N,# 3
    #             N_sig,# 4
    #             N_bak,# 5
    #             rotang_smudge_divs, # 6 determines span that the stamp is smudged
    #             smudge_factor, # 7 factor that multiplies r determining how far the stamp can be smudged
    #             N_bak_smudge, # 8 number of times the back is smudged off its center
    #             n_sig_smudge, # 9 number of times the signal was smudged, if this struct is used as initializer and this number is set to 0 then signal is equalized to background
    #             smudge_sigma_extent #10 how far the smudge is computed out to 
    #             ]
 
    ####
    ###stampKey[
    # 0 ID=0 #stamp number which matches the number of the image file
    # 1 ang=0.0 #angle of line perp to the stamp edge vs line directly pointing left, or just reference angle
    # 2 y=0.0 #target value for this stamp in the nnet node

    # 3 preRot=0.0 #angle that the stamp needs to be rotated before it is processed
    # 4 offset_x=0.0 #offset that the stamp needs to be moved by befor  it is processed
    # 5 offset_y=0.0

    #corner key info
    # 6 R=0.0 #radius of rounded corner
    # 7 opening_ang=0.0 #opening angle for rounded corner
    #
    # 8 inverse flag 0=normal  1=inverse
    #]


    #####


    def __init__(self,
                 eyePlusBufferDim,
                 scale_r,
                 openingAngDivisisor=6.0,
                 radCmul=2.0,#factor by with the radius increases per stamp
                 numAngDiv=12.0,
                 numCircleRadii=7.0,
                 minCircleRadSF=2.0,
                 maxFinalOpeningAng=3.1,
                 numFinalOpeningAngs=6
                 ):
        if(eyePlusBufferDim<=2):
            return
        #set in init
        self.r=scale_r
        halfdim=eyePlusBufferDim #image dim is actually the size plus the buffer that one hexeye extends outwards from its center
        self.imgDim=int(2*eyePlusBufferDim)
        self.stampImgCenter=[halfdim,halfdim]

        self.numAngDiv=numAngDiv
        self.numCircleRadii=int(numCircleRadii)

        self.NFinalOpeningAngs=numFinalOpeningAngs
        self.minCircleRadius = scale_r*minCircleRadSF

        self.cornerOpeningAng = math.pi/openingAngDivisisor
        self.radCmul=radCmul
        self.maxFinalOpeningAng=maxFinalOpeningAng

        #calculating number of stamps, dangs and ansgs
        if(self.numAngDiv<=0.0):
            return
        self.DAng=2.0*math.pi/self.numAngDiv
        self.n_ang = math.floor(self.numAngDiv)
        self.max_total_num_of_stamps=self.n_ang*self.numCircleRadii
        #for each ang/radii combo there is at most a solid stamp and then a series of closing angle stamps
        self.max_total_num_of_stamps*=(1+self.NFinalOpeningAngs)
        #there should also be inverse stamps
        self.max_total_num_of_stamps*=2

        #variables that will be set as the class output
        self.stampN=0 #number of stamps

        #######
        #working scratch
        self.UBasis0=[0.0, 0.0]
        self.UBasis1=[0.0, 0.0]
        self.UrevBasis=[[0.0, 0.0],[0.0, 0.0]] #[[revU0_x, revU1_x] [revU0_y, revU1_y]]
        self.circle_center=[0.0, 0.0]
        self.circle_radius=0.0
        self.line_intersect=[0.0,0.0]
        self.circle_half_pt=[0.0, 0.0] #point in the x direction that is the x coord of the line intercepts
        self.Uline_perp1=[0.0, 0.0] # points perp to line 1 into region between lines
        self.Uline_perp2=[0.0, 0.0]
        self.UcenterIn=[0.0, 0.0] #points inward from center of where lines would intersect, should be set to 1, 0

    ##run
    def run(self, Keys):
        img_arr = self.stampRoundedCornerImgs(Keys)
        self.stampInvImages(img_arr, Keys)
        #for cur_img in img_arr:
        #    cur_img.show()
        return img_arr

    ######################################################################
    ## run functions

    def init_image(self): #img dim is the dim from the center to the edge, full size is twice this
        img_size=(self.imgDim, self.imgDim)
        blank_img=Image.new(mode="RGB", size=img_size)#image defaults to black
        return blank_img

    def stampRoundedCornerImgs(self, Keys):
        center=(0.0, 0.0)
        cur_ang=0.0
        imgArr=[]
        for i_ang in range(self.n_ang):
            cur_circleRadius=self.minCircleRadius
            for i_rad in range(self.numCircleRadii):
                new_img=self.init_image()
                s_Key=[0.0]*g_stamp.s_key_len
                new_img=self.stampImg(new_img, s_Key, center, cur_ang, cur_circleRadius, self.cornerOpeningAng)
                imgArr.append(new_img)
                Keys.append(s_Key)
                #final angle spread
                if cur_circleRadius<=self.stampImgCenter[0] and self.NFinalOpeningAngs>=1:
                    opening_ang_end=self.maxFinalOpeningAng
                    dAng=(opening_ang_end-self.cornerOpeningAng)/self.NFinalOpeningAngs
                    for op_i in range(self.NFinalOpeningAngs):
                        i=self.NFinalOpeningAngs-1-op_i
                        opening_ang=opening_ang_end-i*dAng
                        new_img1=self.init_image()
                        s_Key1=[0.0]*g_stamp.s_key_len
                        new_img1 = self.stampImg(new_img1, s_Key1, center, cur_ang, cur_circleRadius, opening_ang)
                        imgArr.append(new_img1)
                        Keys.append(s_Key1)
                cur_circleRadius *= self.radCmul
            #final image the straight half
            new_img2=self.init_image()
            s_Key2=[0.0]*g_stamp.s_key_len
            new_img2=self.stampHalfImg(new_img2,s_Key2, center) #uses the ang set in stampImg
            imgArr.append(new_img2)
            Keys.append(s_Key2)
            #update and continue loop
            cur_ang+=self.DAng
        return imgArr

    def stampInvImages(self, imgArr, Keys):
        orig_len = len(imgArr)
        for i_stamp in range(orig_len):
            orig_img = imgArr[i_stamp]
            orig_key = Keys[i_stamp]
            new_img = ImageOps.invert(orig_img)
            s_Key=[0.0]*g_stamp.s_key_len
            for i_key in range(g_stamp.s_key_len):
                s_Key[i_key]=orig_key[i_key]
            s_Key[0]=self.stampN
            self.stampN+=1
            s_Key[8]=1.0
            imgArr.append(new_img)
            Keys.append(s_Key)

    def stampImg(self,
                 img,#image obj passed to be modified
                 Key,#key object also modified
                 center,
                 ang,
                 circle_rad,
                 opening_ang):
        self.setBasisFromAng(ang)
        self.setRoundedCorner(center,circle_rad, opening_ang)
        self.renderStampToImg(img)
        #return the stampKey
        Key[0]=self.stampN #this is updated after the stamp is completed
        Key[1]=ang
        Key[6]=circle_rad
        Key[7]=opening_ang
        self.stampN+=1
        return img

    def stampHalfImg(self, img, Key, center):
        self.renderHalfImg(img)
        Key[0]=self.stampN
        self.stampN+=1
        return img

 
    ##################
    #setting class variables that are specific to each stamp
    ####
    #seting Basis for specific Ang
    def setBasisFromAng(self, ang):#angle is in radians
        basisVecs=RoMath.BasisT(ang)
        self.UBasis0=basisVecs[0]
        self.UBasis1=basisVecs[1]
        revBasisVecs = RoMath.revBasisT(self.UBasis0, self.UBasis1)
        self.UrevBasis=RoMath.BasisVecsToMatrix(revBasisVecs[0], revBasisVecs[1])

    def setRoundedCorner(self, center, radius, ang_rad):
        if ang_rad>m_stamp_max_ang_rad:
            return False
        self.circle_radius=radius
        self.UcenterIn[0]=-1.0
        self.UcenterIn[1]=0.0
        offset = [radius*self.UcenterIn[0], radius*self.UcenterIn[1]]
        self.circle_center=[center[0]+offset[0], center[1]+offset[1]]
        halfAng=ang_rad/2.0
        l1=[math.cos(halfAng), math.sin(halfAng)]
        if(l1[0]<=0.0):
            return False
        l2=[l1[0], -l1[1]]
        circle_half_pt = [radius*l1[0], 0.0]
        self.circle_half_pt = [circle_half_pt[0]+offset[0], circle_half_pt[1]+offset[1]]
        self.Uline_perp1[0]=-l1[0]
        self.Uline_perp1[1]=-l1[1]
        self.Uline_perp2[0]=-l2[0]
        self.Uline_perp2[1]=-l2[1]
        Ltanvec=radius*l1[1]/l1[0]
        OutToPt=radius*l1[0]+Ltanvec*l1[1]
        self.line_intersect[1]=center[1]
        self.line_intersect[0]=center[0]+OutToPt-radius
        return True


    ##########################################################
    #rendering stamp to image and helper functions below
    ####################
    def renderStampToImg(self, img):#img will be a Image object
        WH=self.imgDim
        pt=[0.0, 0.0]
        for j in range(WH):
            for i in range(WH):
                img_ij=[i, j]
                pt=self.imgCoordToStampCoord(img_ij)
                intensity = self.stampIntensity(pt)
                intensity*=0xff
                colint=int(intensity)
                curCol=(colint, colint, colint)
                if img_ij[0]>=0:
                    img.putpixel(img_ij, curCol)
        return img

    def renderHalfImg(self, img):
        WH=self.imgDim
        pt=[0.0, 0.0]
        for j in range(WH):
            for i in range(WH):
                img_ij=[i, j]
                pt=self.imgCoordToStampCoord(img_ij)
                intensity=self.halfImgIntensity(pt)
                intensity*=0xff
                colint=int(intensity)
                curCol=(colint, colint, colint)
                if img_ij[0]>=0:
                    img.putpixel(img_ij, curCol)
        return img

    def stampIntensity(self,pt):
        convPt=np.dot(self.UrevBasis,pt)
        intensity = 1.0
        if not self.isInRoundedCornerNoRot(convPt):
            return m_stampZeroIntensity
        return intensity

    def imgCoordToStampCoord(self, img_ij):
        stampCoord=[img_ij[0]-self.stampImgCenter[0], img_ij[1]-self.stampImgCenter[1]]
        return stampCoord

    def halfImgIntensity(self, pt):
        convPt=np.dot(self.UrevBasis,pt)
        intensity=1.0
        if(convPt[0]>0.0):
            return m_stampZeroIntensity
        return intensity

    def isInRoundedCornerNoRot(self,pt):
        isInLines = self.isUnderLine(pt, self.Uline_perp1) and self.isUnderLine(pt, self.Uline_perp2)
        if not isInLines:
            return False
        if self.isInsideCurveHalf(pt):
            return True
        return self.isInCircle(pt)

    def isUnderLine(self,pt, Uline_perp):
        VtoPt =[pt[0]-self.line_intersect[0],pt[1]-self.line_intersect[1]]
        return np.dot(VtoPt, Uline_perp) >= 0.0

    def isInsideCurveHalf(self, pt):
        VtoPt = [pt[0]-self.circle_half_pt[0], pt[1]-self.circle_half_pt[1]]
        return np.dot(VtoPt, self.UcenterIn)>=0.0

    def isInCircle(self, pt):
        VtoPt=[pt[0]-self.circle_center[0], pt[1]-self.circle_center[1]]
        Vmag = np.linalg.norm(VtoPt)
        return self.circle_radius>=Vmag





