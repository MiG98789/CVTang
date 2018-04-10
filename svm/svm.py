#!/usr/bin/python

import sys
import cv2
import json
import os
import numpy as np
import scipy.sparse.linalg as sla
from numpy.linalg import norm
from scipy.linalg import solve

line_mode = 0 #1 for x, 2 for y, 3 for z, -1 for ref_points, -2 for origin, -3 for base_interest_point, -4 for interest_point
vanish_points = [[], [], []]
reference_points = []
reference_length = []
origin_point = []
base_interest_point = []
interest_point = []
line_colors = [(0, 0, 255), (0, 255, 0), (255, 0, 0)]
point_colors = [(255, 255, 0), (255, 0, 255), (0, 255, 255)]
reference_colors = [(255, 102, 153), (51, 255, 102)]
origin_color = (0, 0, 0)

def mouse_callback(event, x, y, flags, prams):
    global line_mode, vanish_points, reference_line, count

    if event == cv2.EVENT_LBUTTONUP:
        if line_mode == 0:
            pass
        elif line_mode == -1:
            if len(reference_points) < 3:
                reference_points.append((x, y))
        elif line_mode == -2:
            if len(origin_point) == 0:
                origin_point.append((x, y))
        elif line_mode == -3:
            if len(base_interest_point) == 0:
                base_interest_point.append((x,y,1))
        elif line_mode == -4:
            if len(interest_point) == 0:
                interest_point.append((x,y,1))
        else:
            vanish_points[line_mode - 1].append((x, y))

    elif event == cv2.EVENT_MBUTTONUP:
        if line_mode == 0:
            pass
        elif line_mode == -1:
            if len(reference_points) > 0:
                del reference_points[-1]
        elif line_mode == -2:
            if len(origin_point) > 0:
                del origin_point[-1]
        elif line_mode == -3:
            if len(base_interest_point) > 0:
                del base_interest_point[-1]
        elif line_mode == -3:
            if len(interest_point) > 0:
                del interest_point[-1]
        elif len(vanish_points[line_mode - 1]) > 0:
            del vanish_points[line_mode - 1][-1]

def normalize(v):
    return np.array(v) / norm(v)

def ssq(a):
    return np.sum(a**2)

def compute_vanish_line(pts, h, w):
    V = [None] * 3

    for dim in range(3):
        pt = np.array([[x, y, 1] for (x, y) in pts[dim]])
        line = np.cross(pt[::2,:], pt[1::2,:])

        M = np.zeros([3,3])
        for d1 in range(3):
            for d2 in range(3):
                M[d1,d2] = np.sum(line[:,d1] * line[:,d2])

        eigval, eigvec = sla.eigs(M, k=1, which='SM')
        eigvec = np.transpose(eigvec.real)
 
        V[dim] = eigvec[-1]/eigvec[-1,-1]

    return V

def compute_scale(V, ref_pts, ref_len, origin, h, w):
    origin = np.array(origin + (1,))
    ref_pt = np.array([pt + (1,) for pt in ref_pts])

    a = [None] * 3
    for dim in range(3):
        a[dim] = np.linalg.lstsq(
                (V[dim] - ref_pt[dim,:])[:,None],
                (ref_pt[dim,:] - origin)[:,None])[0]/ref_len[dim]

    return np.array(a).flatten()

def compute_homography(a, V, O, h, w):
    O = np.array(O + (1,))[:, None]
    aV = a * np.transpose(V)
    P = np.hstack((aV, O))

    Hx = P[:, [0,1,3]]
    Hy = P[:, [1,2,3]]
    Hz = P[:, [0,2,3]]

    H = np.array([Hx, Hy, Hz])
    return H

def transform(image, H):
    images = [None]*3

    for dim in range(3):
        images[dim] = cv2.warpPerspective(image, H[dim], (image.shape[0]*8, image.shape[1]*8), flags=cv2.WARP_INVERSE_MAP)

        gray = cv2.cvtColor(images[dim], cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, 1, 255, cv2.THRESH_BINARY)
        _, contours,_ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if len(contours) > 0:
            cnt = contours[0]
            x,y,w,h = cv2.boundingRect(cnt)
            images[dim] = images[dim][y:y+h, x:x+w]

    return images

def main():
    global line_mode, vanish_points, reference_points, reference_length, origin_point
    image_file = sys.argv[1] if len(sys.argv) > 1 else 'lecture.jpg'
    image = cv2.imread(image_file)
    if image is None:
        print 'Read image file failed'
        sys.exit(1)

    h, w, c = image.shape
    stroke = 3

    cv2.namedWindow('canvas',cv2.WINDOW_NORMAL)
    cv2.resizeWindow('canvas', 900, 900)
    cv2.moveWindow("canvas", 1000,350)
    cv2.setMouseCallback('canvas', mouse_callback)

    while True:
        canvas = image.copy()
        for plane in range(3):
            for idx in range(len(vanish_points[plane])/2):
                cv2.line(canvas, vanish_points[plane][2*idx], vanish_points[plane][2*idx+1], line_colors[plane], 1)
        for plane in range(3):
            for point in vanish_points[plane]:
                cv2.circle(canvas, point, stroke, point_colors[plane], -1)
        for point in reference_points:
            cv2.circle(canvas, point, stroke, reference_colors[1], 2)
        for point in origin_point:
            cv2.circle(canvas, point, stroke, origin_color, 2)
        
        cv2.imshow('canvas', canvas)
        if cv2.getWindowProperty('canvas', 0) < 0:
            sys.exit(0)
        key = cv2.waitKey(1) & 0xff

        #---Specify plane---#
        if key >= ord('x') and key <= ord('z'):
            line_mode = key - ord('x') + 1

        elif key == ord('r'):
            line_mode = -1

        elif key == ord('o'):
            line_mode = -2

        elif key == ord('d'):
            sys.stdout.write('Input reference lengths along x, y, z respectively: ')
            sys.stdout.flush()
            try:
                reference_length = [float(x) for x in raw_input().split(' ')]
                print 'Lengths recorded'
            except:
                print 'Invalid input'

        elif key == 13:
            if all([len(v) >= 4 for v in vanish_points]):
                V = compute_vanish_line(vanish_points, h, w)
                print 'Vx', V[0]
                print 'Vy', V[1]
                print 'Vz', V[2]

                if len(reference_points) >= 3 and len(origin_point) == 1:
                    if len(reference_length) == 0:
                        sys.stdout.write('Input reference lengths along x, y, z respectively: ')
                        sys.stdout.flush()
                        try:
                            reference_length = [float(x) for x in raw_input().split(' ')]
                            print 'Lengths recorded'
                        except:
                            print 'Invalid input'
                    print 'Rx', reference_length[0]
                    print 'Ry', reference_length[1]
                    print 'Rz', reference_length[2]

                    a = compute_scale(V, reference_points, reference_length, origin_point[0], h, w)
                    print 'ax', a[0]
                    print 'ay', a[1]
                    print 'az', a[2]

                    H = compute_homography(a, V, origin_point[0], h, w)
                    print 'Hxy', H[0]
                    print 'Hyz', H[1]
                    print 'Hxz', H[2]
                    
                    images = transform(image, H)
                    cv2.imshow('xy', images[0])
                    cv2.imshow('yz', images[1])
                    cv2.imshow('xz', images[2])
                    print 'Transformed'
                else:
                    print 'Reference points definition not complete'
            else:
                print 'Axis points definition not complete'

        elif key == ord('i'):
            select_interest_points = 'y'
            three_d_points = []
            texture_points = []

            while select_interest_points.lower() == 'y':
                line_mode = -3
                if len(base_interest_point) > 0:
                    del base_interest_point[-1]
                print 'Select a base point.'
                while len(base_interest_point) == 0:
                    cv2.waitKey(1)
                print 'Base point at %s' % (base_interest_point[0],)

                line_mode = -4
                if len(interest_point) > 0:
                    del interest_point[-1]
                print 'Select an interest point.'
                while len(interest_point) == 0:
                    cv2.waitKey(1)
                print 'Added interest point at %s' % (interest_point[0],)

                line1 = np.cross(base_interest_point[0], np.array([origin_point[0][0], origin_point[0][1], 1]))
                
                horizon = np.cross(V[1], V[0])
                horizon = horizon/np.sqrt(horizon[0]**2 + horizon[1]**2)
                
                v = np.cross(line1, horizon)
                v = np.divide(v, v[2])

                line2 = np.cross(np.transpose(v), np.array([reference_points[2][0], reference_points[2][1], 1]))

                vertical_line = np.cross(interest_point[0], base_interest_point[0])

                t = np.cross(line2, vertical_line)
                t = np.divide(t, t[2])

                height = reference_length[2] \
                        * np.sqrt(ssq(np.subtract(interest_point[0], base_interest_point[0]))) \
                        * np.sqrt(ssq(np.subtract(np.transpose(V[2]), t))) \
                        / np.sqrt(ssq(np.subtract(t, base_interest_point))) \
                        / np.sqrt(ssq(np.subtract(V[2], interest_point[0])))
                print 'Height: %s' % (height,)

                V = compute_vanish_line(vanish_points, h, w)
                a = compute_scale(V, reference_points, reference_length, origin_point[0], h, w)
                O = np.array(origin_point[0] + (1,))[:, None]
                aV = a * np.transpose(V)
                P = np.hstack((aV, O))
                
                Hz = np.transpose([P[:,0], P[:,1], np.add(P[:,2]*height, P[:,3])])
                pos = solve(Hz, interest_point[0])
                three_d_points.append([pos[0]/pos[2], pos[1]/pos[2], height])
                print '3D coordinates: %s' % (three_d_points[-1],)

                add_more = raw_input('Continue adding points at the same height? (y/n) ')
                while add_more.lower() == 'y':
                    if len(interest_point) > 0:
                        del interest_point[-1]
                    print 'Select an interest point at the same height.'
                    while len(interest_point) == 0:
                        cv2.waitKey(1)
                    pos = solve(Hz, interest_point[0])
                    three_d_points.append([pos[0]/pos[2], pos[1]/pos[2], height])
                    print '3D coordinates: %s' % (three_d_points[-1],)
                    add_more = raw_input('Continue adding points at the same height? (y/n) ')

                select_interest_points = raw_input('Add more interest points? (y/n) ')

            print 'All 3D points added:'
            print '\n'.join(str(point) for point in three_d_points)
            line_mode = 0

            texture_file = raw_input('Input texture map file name: ')
            texture = cv2.imread(texture_file)
            if texture is None:
                print 'Read image file failed'
                sys.exit(1)
            
            texture_name = texture_file[::-1].split('.', 1)[-1].split('/')[0][::-1]
            texture_height, texture_width, _ = texture.shape
            
            for three_d_point in three_d_points:
                sys.stdout.write('Input texture map coordinates for %s: ' % (three_d_point,))
                sys.stdout.flush()
                texture_point = []
                try:
                    texture_point = [float(x) for x in raw_input().split(' ')]
                    print 'Texture pixel point recorded'
                except:
                    print 'Invalid input'
                texture_points.append([texture_point[0]/texture_width, (texture_height - texture_point[1])/texture_height])
                print 'Adding texture point at %s' % (texture_points[-1],)
            print 'All texture points added:'
            print '\n'.join(str(point) for point in texture_points)

            image_name = image_file[::-1].split('.', 1)[-1].split('/')[0][::-1]
            wrl_file = 'model/' + image_name + '.txt'
            if not os.path.exists(wrl_file):
                with open(wrl_file, 'w') as f:
                    f.write('#VRML V2.0 utf8\n\nCollision {\n collide FALSE\n children [\n ]\n }')
            lines = ''
            with open(wrl_file, 'r') as f:
                lines = f.readlines()
            with open(wrl_file, 'w') as f:
                for line in lines[:-2]:
                    f.write(line)
            with open(wrl_file, 'a') as f:
                f.write('\n Shape {\n  appearance Appearance {\n   texture ImageTexture {\n   url "')
                f.write(texture_name + '.gif')
                f.write('"\n  }  \n  }\n   geometry IndexedFaceSet {\n   coord Coordinate {\n   point [\n')
                for point in three_d_points:
                    f.write('     ' + str(point[0]) + ' ' + str(point[1]) + ' ' + str(point[2]) + ', \n')
                f.write('\n   ]\n   }\n   coordIndex [\n    ')
                for i in range(len(three_d_points)):
                    f.write(str(i) + ',')
                f.write('-1')
                f.write('\n   ]\n   texCoord TextureCoordinate {\n    point [\n')
                for point in texture_points:
                    f.write('     ' + str(point[0]) + ' ' + str(point[1]) + ', \n')
                f.write('\n    ]\n   }\n   texCoordIndex [\n    ')
                for i in range(len(texture_points)):
                    f.write(str(i) + ',')
                f.write('-1')
                f.write('\n   ]\n   solid FALSE\n  }\n}')
                f.write('\n ] \n}')
        #---#

        #---Utils---#
        elif key == ord('q'):
            line_mode = 0
        elif key == ord('c'):
            vanish_points = [[], [], []]
            reference_points = []
            query_points = []
            origin_point = []
            base_interest_point = []
            interest_point = []
        elif key == ord(' '):
            stroke = 3 if stroke == 10 else 10

        elif key == 27:
            sys.exit(0)
        elif key == 8:
            sys.stdout.write('\033[H\033[J')
            sys.stdout.flush()
        #---#

        #---Load and save---#
        elif key == ord('s'):
            data = {'points': vanish_points, 'ref_points': reference_points, 'ref_lens': reference_length, 'origin_point': origin_point}
            with open(image_file.split('.')[0] + '.json', 'w') as f:
                json.dump(data, f, indent=4)
            print 'Vanishing points saved'
        elif key == ord('l'):
            with open(image_file.split('.')[0] + '.json', 'r') as f:
                data = json.load(f)
            vanish_points = [[tuple(y) for y in x] for x in data['points']]
            reference_points = [tuple(x) for x in data['ref_points']]
            reference_length = data['ref_lens']
            origin_point = [tuple(x) for x in data['origin_point']]
        #---#

if __name__ == "__main__":
    main()
