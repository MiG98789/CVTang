#!/usr/bin/python

import os
import cv2
import numpy as np
import matplotlib.pyplot as plt

from mpl_toolkits.mplot3d import Axes3D

def load_light(dirname):
    with open(dirname + '/lightvec.txt', 'r') as f:
        lights = f.readlines()

    lights = [[float(x) for x in y.split(' ')] for y in lights]
    lights = np.array(lights)
    return lights

def load_image(dirname):
    dirs = sorted(os.listdir(dirname))
    dirs = [x for x in dirs if 'image' in x]

    images = [cv2.imread(dirname + '/' + d) for d in dirs]

    return np.array(images)

def vis_light(lights):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    ax.plot(lights[:,0], lights[:,1], lights[:,2])
    plt.show()

def vis_poly(verts, faces):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    ax.scatter(verts[:,0], verts[:,1], verts[:,2])
    plt.show()

def subdivide(verts, faces):
    def normalize(v1, v2):
        v = [v1[x]/2. + v2[x]/2. for x in range(len(v1))]
        norm = np.linalg.norm(v)
        return v if norm == 0. else v/norm

    for f in range(len(faces)):
        face = faces[f]
        v1, v2, v3 = (verts[x] for x in face)

        #Add subdivided vertices calculated from 3 edges
        verts.append(normalize(v1,v2))
        verts.append(normalize(v2,v3))
        verts.append(normalize(v3,v1))

        #Define newly defined faces
        i = len(verts) - 3
        j, k = i+1, i+2
        faces.append((i, j, k))
        faces.append((face[0], i, k))
        faces.append((i, face[1], j))
        faces[f] = (k, j, face[2])

    return verts, faces

def resample(images, lights):
    def sqrt(x): return x**0.5
    def qurt(x): return x**0.25

    #Define golden ratio and vertex values
    t = (1 + sqrt(5))/2.0
    a = sqrt(t)/qurt(5)
    b = 1/(sqrt(t) * qurt(5))

    #Constructs icosahedron
    verts = [[-b, 0, a], [ b, 0, a], [-b, 0,-a], [ b, 0,-a],
             [ 0, a, b], [ 0, a,-b], [ 0,-a, b], [ 0,-a,-b],
             [ a, b, 0], [-a, b, 0], [ a,-b, 0], [-a,-b, 0]]

    faces = [[0, 4, 1], [0,9, 4], [9, 5,4], [ 4,5,8], [4,8, 1],    
             [8,10, 1], [8,3,10], [5, 3,8], [ 5,2,3], [2,7, 3],    
             [7,10, 3], [7,6,10], [7,11,6], [11,0,6], [0,1, 6], 
             [6, 1,10], [9,0,11], [9,11,2], [ 9,2,5], [7,2,11]]

    #Subdivides faces
    for i in range(4):
        verts, faces = subdivide(verts, faces)
    
    Los = np.array(verts)
    Los = Los[Los[:,2] >= 0]
    L = lights
    imgs = []
    for Lo in Los:
        #Finds closest lights
        V = np.argpartition(np.linalg.norm(L-Lo, axis = 1), 3)[:3]

        #Interpolates images from closest lights
        denom = np.sum([np.dot(Lo, L[i]) for i in V])
        Io = [np.dot(Lo, L[i])/denom * images[i] for i in V]
        imgs.append(np.sum(Io, axis = 0).astype(np.uint8))

    return np.array(imgs), Los
   
def denom_image(images, lights):
    n, h, w, c = images.shape
    images = images.reshape(n, -1, c)

    #sorted images
    sorted_img = np.sort(images, axis=0)
    
    #get L H threshold
    L = np.percentile(sorted_img, 70, axis=0).reshape(1, -1, c)
    H = np.percentile(sorted_img, 90, axis=(0,1))

    #find images that satisfy kiL > L
    cond = np.all(images - L > 0, axis = 2)

    #get kiL and riL
    kiL = np.sum(cond, axis=1)
    riL = (images - L)*cond[:,:,None]
    bot = np.count_nonzero(riL, axis=1)
    riL = np.divide(np.sum(riL, axis=1), bot, where=bot!=0)

    #Finds denominator image that maximizes k and has r < H
    cond = np.zeros(kiL.shape)
    cond[np.all(riL < H)] = kiL

    index = np.argmax(cond)
    denom_img = images[index]
    denom_lit = lights[index]
 
    images = np.divide(images, denom_img, where = denom_img!=0)
    images = np.delete(images, index, axis=0).reshape(-1, h, w, c)

    lights = np.divide(lights, denom_lit, where = denom_lit!=0)
    lights = np.delete(lights, index, axis=0)

    return images, lights

def estimate_normal(images, lights):
    imgray = [cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) for img in images]
    imgray = [img.flatten() for img in imgray]
    imgray = np.array(imgray)

    G, _, _, _ = np.linalg.lstsq(lights, imgray, rcond=None)
    G = G.T

    kd = np.linalg.norm(G, axis = 1)
    N = G / kd[:, None]
    return N

def main():
    data_path = 'data02'
    lights = load_light(data_path)
    images = load_image(data_path)

    images, lights = resample(images, lights)
    images, lights = denom_image(images, lights)
    
    normal = estimate_normal(images, lights)

if __name__ == "__main__":
    main()
