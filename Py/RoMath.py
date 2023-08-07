import math

def angTo2Pi(angIn):
    ang=angIn
    twoPi=2.0*math.pi
    while ang>= twoPi:
        ang-=twoPi
    while ang<0.0:
        ang+=twoPi
    return ang

def BasisT(angRad): #gives basis for rad but final tuple is two seperate vectors
                    # the first is the Ux and the second is the Uy of the basis
                    #the tuple should not be interpreted as a matrix for this basis
    basisU0=[0.0, 0.0]
    basisU1=[0.0, 0.0]
    basisU0[0]=math.cos(angRad)
    basisU0[1]=math.sin(angRad)
    basisU1[0]=-math.sin(angRad)
    basisU1[1]=math.cos(angRad)
    return basisU0, basisU1

def revBasisT(basisU0, basisU1):#reverses the vectors from BasisT and returns the 
                                #results in the same format: first vector is Ux, second is Uy
    revBasisU0=[0.0, 0.0]
    revBasisU1=[0.0, 0.0]
    revBasisU0[0]=basisU1[1]
    revBasisU0[1]=-basisU0[1]
    revBasisU1[0]=-basisU1[0]
    revBasisU1[1]=basisU0[0]
    return revBasisU0, revBasisU1

def BasisVecsToMatrix(BasisU0, BasisU1):#converts a set of vectors from BasisT to a good matrix
                                  #for the basis
    Basis=[[0.0, 0.0], [0.0, 0.0]]
    #set this to ((U0_x, U1_x)
    #             (U0_y, U1_y)
    Basis[0][0]=BasisU0[0]#U0_x
    Basis[1][0]=BasisU0[1]#U0_y
    Basis[0][1]=BasisU1[0]#U1_x
    Basis[1][1]=BasisU1[1]#U1_y
    return Basis