from __future__ import division
import sys
import yaml
import threading
import argparse
import logging
import cv2
import random
import time
import datetime
import os 
from matplotlib import pyplot as plt
from os import listdir
from os.path import isfile, join, isdir

import numpy as np
from PIL import Image
import src.siamese as siam
from src.tracker import tracker
from src.parse_arguments import parse_arguments
from src.region_to_bbox import region_to_bbox
#from myCamera import getImages

from darknet import *
import database
import math

colors = [(255,0,0), (0,255,0), (0,0,255), (0,255,255), (255,255,0), (255,0,255)]

class siamfcGraph:
    def __init__(self,filename,image,templates_z,scores):
        self.filename = filename
        self.image = image
        self.templates_z = templates_z
        self.scores = scores

def getImages(folder_name,imagesPerSec,totalTime,database, now, graph, scfg):
    fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    cam = cv2.VideoCapture(0)
    yoloList = [threading.Thread()]
    for timeStep in range(0,totalTime):
        if(timeStep != 0):
            yoloList.append(threading.Thread())

        new_dir = os.path.join(folder_name,'%d_%d_%d'% (now.hour, now.minute, now.second+timeStep))
        #print(new_dir)
        if not os.path.exists(new_dir):
            os.makedirs(new_dir)
        
        f = open(new_dir+"/groundtruth.txt","w+")
        for i in range(0,imagesPerSec):
            ret, frame = cam.read()
            #cv2.imshow("test", frame)
            if not ret:
                break
            k = cv2.waitKey(int(1000/imagesPerSec))
            img_name = new_dir+"/opencv_frame_%03d.png" % i
            cv2.imwrite(img_name, frame)
            #print("{} written!".format(img_name))
            f.write(("100 100 100 100\n"))

        height, width = frame.shape[:2]
        yoloList[timeStep] = threading.Thread(target=runYolo,args=(new_dir,database, fourcc, width, height,graph, scfg))
        #print('Yolo started')
        yoloList[timeStep].start();        
        f.close()
    cam.release()
    for i in yoloList:
        if(i.is_alive()):
            i.join()

    return

def CameraAvailable(oldThread,newThread):
    oldThread.join()
    newThread.start()

def showYoloResult(folderPath, YoloVid, show):
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if isfile(join(folderPath,f)) and (f.endswith("yolo.jpg") or f.endswith("yolo.png"))]
    imageFiles.sort()
    img = None
    for i in imageFiles:
        #print(i)
        im = plt.imread(i)
        imCV = cv2.imread(i,1)
        if(show):
            if img is None:
                img = plt.imshow(im)
            else:
                img.set_data(im)
            plt.pause(.1)
            plt.draw()
        YoloVid.write(imCV)

def showSiamFCResult(folderPath, SiamfcVid, show):
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if isfile(join(folderPath,f)) and (f.endswith("siamfc.jpg") or f.endswith("siamfc.png"))]
    imageFiles.sort()
    img = None
    for i in imageFiles:
        im = plt.imread(i)
        imCV = cv2.imread(i,1)
        if(show):
            if img is None:
                img = plt.imshow(im)
            else:
                img.set_data(im)
            plt.pause(.1)
            plt.draw()
        SiamfcVid.write(imCV)

def runYolo(folderPath, database, fourcc, testWidth, testHeight, graph, scfg):
    t_start = time.time()
    #print(folderPath)
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if (isfile(join(folderPath,f)) and f.endswith(".png") and not f.endswith("yolo.png"))]
    imageFiles.sort()
    imgNr = 0;
    YoloVid = cv2.VideoWriter(join(folderPath,'Yolov3Vid.avi'),fourcc,10,(testWidth,testHeight))
    f = open(folderPath+"/YoloBoxes.txt","w+")
    for singleImage in imageFiles:
        #print(singleImage)
        r = detect(database.net, database.meta, singleImage.encode('utf8'))
        img = cv2.imread(singleImage,1)
        colorCounter = 0;
        boxNr = 0
        for yoloBB in r:
            if(boxNr == 0):
                f.write('%s,%d,%d,%d,%d'%(yoloBB[0],yoloBB[2][0],yoloBB[2][1],yoloBB[2][2],yoloBB[2][3]))
            else:
                f.write(':%s,%d,%d,%d,%d'%(yoloBB[0],yoloBB[2][0],yoloBB[2][1],yoloBB[2][2],yoloBB[2][3]))
            boxNr = boxNr+1
            cv2.rectangle(img,(int(yoloBB[2][0]-yoloBB[2][2]/2),int(yoloBB[2][1]-yoloBB[2][3]/2)),
                (int(yoloBB[2][0]+yoloBB[2][2]/2),int(yoloBB[2][1]+yoloBB[2][3]/2)), colors[colorCounter],3)
            colorCounter = colorCounter+1
            if(colorCounter >= len(colors)):
                colorCounter = 0
        f.write('\n')
        imgNr = imgNr+1
        cv2.imwrite(singleImage[0:-4]+"_yolo.png",img)
        #print(singleImage[0:-4]+"_yolo.png is written")
        YoloVid.write(img)
    t_elapsed = time.time() - t_start
    #print('FPS = '+ str(len(imageFiles)/t_elapsed))
    f.close()
    siamfcThread = threading.Thread(target=runSiamfc,args=(folderPath,fourcc,testWidth,testHeight, graph, scfg))
    siamfcThread.start()
    YoloVid.release()
    siamfcThread.join()
    return

def runSiamfc(folderPath, fourcc, testWidth, testHeight, graph, scfg):
    print('Running Siamfc: ' + folderPath)
    frame_name_list = _init_video(folderPath)
    hp, evaluation, run, env, design = parse_arguments()
    final_score_sz = hp.response_up * (design.score_sz - 1) + 1
    fp = open(os.path.join(folderPath,'YoloBoxes.txt'))
    filename, image, templates_z, scores, graph1, scfg1 = siam.build_tracking_graph(final_score_sz, design, env)
    finalImages = []
    Allbboxes = []
    SiamfcVid = cv2.VideoWriter(join(folderPath,'SiamfcVid.avi'),fourcc,10,(testWidth,testHeight))
    f = open(folderPath+"/SiamfcBoxes.txt","w+")
    for i,line in enumerate(fp):
        if(line == '\n'):
            continue
        else:
            boxes = line[:-1].split(':')
            boxNr = 0
            for j in boxes:
                box = j.split(',')
                label = box[0]
                box = map(int,box[1:])
                print('In folder %s Image %d has a box at %d,%d,%d,%d with label %s'%(folderPath,i,box[0],box[1],box[2],box[3],label))
                pos_x = box[0]
                pos_y = box[1]
                target_w = box[2]
                target_h = box[3]
                print('Pos_x: %d, Pos_y:%d, width:%d, height:%d'%(pos_x,pos_y,target_w,target_h))
                bboxes, speed, finalImages = tracker(graph1, scfg1, hp, run, design, frame_name_list, pos_x, pos_y, target_w, target_h, final_score_sz,
                                                    filename, image, templates_z, scores, label,0,colors[boxNr%len(colors)],0,9, finalImages, 0)
                Allbboxes.append(bboxes)
                boxNr = boxNr +1
                print(bboxes)
            fname = 0
            probs = [0]*len(Allbboxes)
            #print(Allbboxes)
            oldFolderPath = folderPath.split('_')
            oldFolderPath[2] = str(int(oldFolderPath[2]) - 1)
            underscore = '_'
            oldFolderPath = underscore.join(oldFolderPath)
            if(os.path.isfile(oldFolderPath+"/SiamfcBoxes.txt")):
                print('Extracting old probs')
                probs = getOldProbs(oldFolderPath, Allbboxes, [(item.split(','))[0] for item in boxes])
                print(probs)
            for j in finalImages:
                print(probs)
                calcProbs(j,90,Allbboxes,fname, probs, f,[(item.split(','))[0] for item in boxes])
                SiamfcVid.write(j)
                cv2.imwrite(frame_name_list[fname]+'_siamfc.png',j)
                fname = fname + 1
            break
    f.close()
    SiamfcVid.release()
    return

def calcProbs(img, sourceAngle, allBoxes, imgNum, probs, textFile, labels):
    height, width = img.shape[:2]
    partitions = []
    sourcePixel = int(math.cos(sourceAngle*math.pi/180.0)/2.0*width+width/2)
    pVal = int(sourcePixel/(width/12))
    std1 = []
    std2 = []
    allstd1 = 0
    allstd2 = 0
    for j in range(0,195,15):
        xcord = int(math.cos(j*math.pi/180.0)/2.0*width)
        if(xcord < 0):
            xcord = xcord + 1
        else:
            xcord = xcord - 1
        partitions.append(width/2+xcord)
        std1.append(0)
        std2.append(0)
        cv2.line(img,(int(width/2+xcord),0),(int(width/2+xcord),height-1),(255,0,255),2)

    probsSum = 0
    for i in range(0,len(allBoxes)):
        thisBB = allBoxes[i][imgNum]
        position = int((thisBB[0]+thisBB[2]/2)/(width/12))
        probsSum = probsSum + probs[i]
        if(abs(position-pVal) < 1):
            std1[i] = 1
            std2[i] = 1
            allstd1 = allstd1+1
            allstd2 = allstd2+1
        elif(abs(position-pVal) < 2):
            std2[i] = 1
            allstd2 = allstd2 + 1
    probsSum = probsSum+1
    lineToWrite = ''
    for i in range(0,len(allBoxes)):
        if(allstd2 > 0 and allstd1 > 0):
            probs[i] = probs[i]+std1[i]*.682/allstd1+std2[i]*.318/allstd2
        cv2.rectangle(img,(int(allBoxes[i][imgNum][0]+allBoxes[i][imgNum][2]/2),int(allBoxes[i][imgNum][1]+allBoxes[i][imgNum][3]/2+20)), 
        (int(allBoxes[i][imgNum][0]+allBoxes[i][imgNum][2]/2+100),int(allBoxes[i][imgNum][1]+allBoxes[i][imgNum][3]/2-30)),(255,255,255),-1)
        cv2.putText(img,'%f'%(probs[i]/probsSum),(int(allBoxes[i][imgNum][0]+allBoxes[i][imgNum][2]/2),int(allBoxes[i][imgNum][1]+allBoxes[i][imgNum][3]/2)),cv2.FONT_HERSHEY_SIMPLEX,.5,(0,0,0))
        if i != 0:
            lineToWrite = lineToWrite + '|'
        lineToWrite  = lineToWrite+labels[i]+':'+str(probs[i])+':'+str(allBoxes[i][imgNum][0])+','+str(allBoxes[i][imgNum][1])+','+str(allBoxes[i][imgNum][2])+','+str(allBoxes[i][imgNum][3])
    lineToWrite = lineToWrite+'\n'
    textFile.write(lineToWrite)
    return

def getOldProbs(folderPath, allBoxes, labels):
    rProbs = [0]*len(allBoxes)
    f = open(folderPath+'/SiamfcBoxes.txt','r')
    flines = f.readlines()
    if(len(flines) < 1):
        return rProbs
    finalProb = flines[-1]
    objects = finalProb.split('|')
    oldLabels = [(lbls.split(':'))[0] for lbls in objects]
    oldProbs = [float((lbls.split(':'))[1]) for lbls in objects]
    oldBoxes = [(lbls.split(':'))[2] for lbls in objects]
    for i in range(0,len(oldBoxes)):
        oldBoxes[i] = map(float,oldBoxes[i].split(','))
    for i in range(0,len(allBoxes)):
        maxIntersection = 0
        #A1 = abs(allBoxes[i][0][2]*allBoxes[i][0][3])
        for j in range(0, len(oldLabels)):
            #A2 = abs(float((oldBoxes.split(','))[2])*float((oldBoxes.split(',')[3])))
            if(labels[i] == oldLabels[j]):
                print('Checking box')
                w = max(allBoxes[i][0][0],oldBoxes[j][0])-min(allBoxes[i][0][0]+allBoxes[i][0][2],oldBoxes[j][0]+oldBoxes[j][2])
                h = max(allBoxes[i][0][1],oldBoxes[j][1])-min(allBoxes[i][0][1]+allBoxes[i][0][3],oldBoxes[j][1]+oldBoxes[j][3])
                Aintersection = w*h 
                if(Aintersection > maxIntersection):
                    print('Found match')
                    maxIntersection = Aintersection
                    rProbs[i] = oldProbs[j]
    
    return rProbs

def lols():
    parser = argparse.ArgumentParser(description="Run WIND Project")
    parser.add_argument('-c','--clear',action='store_true', default = False, help='Delete old camera data')
    args = parser.parse_args()
    if(args.clear == True):
        for oldData in os.listdir('CameraData'):
            filep = os.path.join('CameraData',oldData)
            if(os.path.isfile(filep)):
                os.remove(filep)
                print('Deleted: '+ filep)
            elif os.path.isdir(filep):
                for reallyOldData in os.listdir(filep):
                    newfilep = os.path.join(filep,reallyOldData)
                    if(os.path.isfile(newfilep)):
                        os.remove(newfilep)
                os.rmdir(filep)
                print('Deleted: ' + filep + '/')

        print('Done Deleting')
    

    # Initialize object detector
    database.net = load_net(b"YoloConfig/yolov3-tiny.cfg", b"YoloConfig/yolov3-tiny.weights", 0)
    database.meta = load_meta(b"YoloConfig/coco.data")

    # avoid printing TF debugging information
    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
        
    # TODO: allow parameters from command line or leave everything in json files?
    hp, evaluation, run, env, design = parse_arguments()
    final_score_sz = hp.response_up * (design.score_sz - 1) + 1
    
    # build TF graph once for all
    filename, image, templates_z, scores, graph, scfg = siam.build_tracking_graph(final_score_sz, design, env)
    #sFCgraph = siamfcGraph(filename, image, templates_z, scores)
    
    print(filename)
    dirList = [""]
    testImage = Image.open("opencv_frame_000.png")
    testWidth, testHeight  = testImage.size
    print(testWidth,testHeight)
    fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    foreplay = 1
    finalImages = []
    HellifIknow = 0
    clipLength = 10
    plt.xticks([]),plt.yticks([])
    YoloVid = cv2.VideoWriter('Yolov3Vid.avi',fourcc,10,(testWidth,testHeight))
    SiamfcVid = cv2.VideoWriter('SiamfcVid.avi',fourcc,10,(testWidth,testHeight))
    i = 0
    notDone = True

    VideoLength = 3
    now = datetime.datetime.now()
    haha = threading.Thread(target=getImages,args=('CameraData/',10,VideoLength,database,now, graph, scfg))
    haha.start()
    haha.join()
    frame_name_list = _init_video('CameraData/%d_%d_%d/'%(now.hour,now.minute,now.second))
    while notDone and i < VideoLength and datetime.datetime.now()<now+datetime.timedelta(seconds=20+VideoLength):
        try:
            if os.path.isdir(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i))):
                showYoloResult(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i)),YoloVid, False)
                showSiamFCResult(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i)),SiamfcVid, True)
                i = i+1
            else:
                time.sleep(.2)
        except(KeyboardInterrupt, SystemExit):
            notDone = False

    #haha.join()
    YoloVid.release()
    SiamfcVid.release()


def _init_video(folderPath):
    video_folder = folderPath
    frame_name_list = [f for f in os.listdir(video_folder) if (f.endswith(".jpg") or f.endswith(".png") and not('yolo' in f))]
    frame_name_list = [os.path.join(video_folder, '') + s for s in frame_name_list]
    frame_name_list.sort()

    return frame_name_list


if __name__ == '__main__':
    sys.exit(lols())
