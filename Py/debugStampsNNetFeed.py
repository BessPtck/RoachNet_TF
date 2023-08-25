import math
import matplotlib.pyplot as plt
import g_stamp
import StampsNNetFeed
import os
from PIL import Image, ImageDraw

def histRandPt(Npts=4000, Nbins=100, sigma=14.0, max_in_sigmas=3.0):
    sN = StampsNNetFeed.gaussianInt(sigma, 0.0, max_in_sigmas)
    randPts=[0]*Npts
    for i in range(Npts):
        randPts[i]=sN.randPt()
    plt.style.use('_mpl-gallery')
    fig, ax = plt.subplots()
    ax.hist(randPts, bins=Nbins)
    plt.show()

def histRandPt2D(Npts=4000, Nbins=100, sigma=14.0, max_in_sigmas=3.0):
    sN = StampsNNetFeed.gaussianInt(sigma, 0.0, max_in_sigmas)
    randPts=[0]*Npts
    for i in range(Npts):
        randPts[i]=sN.randPt2D()
    plt.style.use('_mpl-gallery')
    fig, ax = plt.subplots()
    ax.hist(randPts, bins=Nbins)
    plt.show()

def histRand2D(Npts=4000, Nbins=20, sigma=14.0, max_in_sigmas=3.0, axlimit_in_sigma=3.0):
    sN=StampsNNetFeed.gaussianInt(sigma, 0.0, max_in_sigmas)
    randPtsX=[0]*Npts
    randPtsY=[0]*Npts
    for i in range(Npts):
        offset=sN.rand2D()
        randPtsX[i]=offset[0]
        randPtsY[i]=offset[1]
    plt.style.use('_mpl-gallery-nogrid')
    fig,ax=plt.subplots()
    xyBins=[Nbins, Nbins]
    ax.hist2d(randPtsX, randPtsY, bins=xyBins)
    axlim=sigma*axlimit_in_sigma
    ax.set(xlim=(-axlim,axlim), ylim=(-axlim,axlim))
    plt.show()
    
def showDirPlots(fdir):
    src_files=os.listdir(fdir)
    for file_name in src_files:
        src_path=fdir+"/"+file_name
        if os.path.isfile(src_path) and file_name.find(g_stamp.imgFileSufix)>1:
            img=Image.open(src_path)
            img.show()

def showImgs():
    fdir="Dat/L0"
    showDirPlots(fdir)