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

class sfc_bbox:
    def __init__(self,color,label,positions,prob):
        self.color = color
        self.label = label
        self.positions = positions
        self.prob = prob
        return

    def padfront(self,numRows):
        if(numRows == 0):
            return
        newEntries = []
        for i in range(numRows):
            newEntries.append([-1, -1, -1, -1])
        self.positions = np.concatenate((newEntries,self.positions),0)
        return

    def padafter(self,numRows):
        if(numRows == 0):
            return
        newEntries = []
        for i in range(numRows):
            newEntries.append([-1, -1, -1, -1])
        self.positions = np.concatenate((self.positions,newEntries),0)
        return

class siamfcGraph:
    def __init__(self,filename,image,templates_z,scores):
        self.filename = filename
        self.image = image
        self.templates_z = templates_z
        self.scores = scores

def getImages(folder_name,imagesPerSec,totalTime,database, now, graph, scfg):
    fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    try:
        cam = cv2.VideoCapture(cameraNumber)
    except:
        print("Camera number given is not valid or connected")

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
        if(doYolo):
            yoloList[timeStep] = threading.Thread(target=runYolo,args=(new_dir,database, fourcc, width, height,graph, scfg))
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
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if isfile(join(folderPath,f)) and (f.endswith("yolo.jpeg") or f.endswith("yolo.png")) and (not 'siamfc' in f)]
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
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if isfile(join(folderPath,f)) and ('siamfc' in f)]
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
    imageFiles = [join(folderPath,f) for f in listdir(folderPath) if (isfile(join(folderPath,f)) 
        and (f.endswith(".png") or f.endswith(".jpeg")) and not f.endswith("yolo.png") and not f.endswith("yolo.jpeg"))]
    imageFiles.sort()
    imgNr = 0;
    YoloVid = cv2.VideoWriter(join(folderPath,'Yolov3Vid.avi'),fourcc,10,(testWidth,testHeight))
    f = open(folderPath+"/YoloBoxes.txt","w+")

    counter = 0
    toolbar_width = 40
    sys.stdout.write("[%s]"%(" "*toolbar_width))
    sys.stdout.flush()
    sys.stdout.write("\b" * (toolbar_width+1))
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
        counter = counter + 1
        sys.stdout.write("[")
        sys.stdout.write("*"*int(counter/len(imageFiles)*toolbar_width))
        sys.stdout.write(" "*int((1-counter/len(imageFiles))*toolbar_width))
        sys.stdout.write("]")
        sys.stdout.write("%d/%d"%(counter, len(imageFiles)))
        sys.stdout.write("\b" * int(toolbar_width+4+min(1,counter/10)+min(1,counter/100)+min(1,counter/1000)+min(1,len(imageFiles)/10)+min(1,len(imageFiles)/100)+min(1,len(imageFiles)/1000)))
    sys.stdout.write("\n")
    t_elapsed = time.time() - t_start
    #print('FPS = '+ str(len(imageFiles)/t_elapsed))
    f.close()
    #if(liveFeed and doSiamfc):
        #siamfcThread = threading.Thread(target=runSiamfc,args=(folderPath,fourcc,testWidth,testHeight, graph, scfg))
        #siamfcThread.start()
        #siamfcThread.join()
    YoloVid.release()
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
    nucAngles = open(os.path.join(folderPath,"nuclearAngles.txt"),"r")
    print(refreshRate)
    for i in range(len(frame_name_list)):
        line = fp.readline()
        finalImages = []
        if(line == '\n'):
            continue
        elif(i%refreshRate == 0):
            boxes = line[:-1].split(':')
            boxNr = 0
            for j in Allbboxes:
                #label = j.label
                #pos_x = j.positions[len(j.positions)-1][0]
                #pos_y = j.positions[len(j.positions)-1][1]
                #target_w = j.positions[len(j.positions)-1][2]
                #target_h = j.positions[len(j.positions)-1][3]
                #bboxes, speed, finalImages = tracker(graph1, scfg1, hp, run, design, frame_name_list[i:i+refreshRate-1], pos_x, pos_y, target_w, target_h, final_score_sz,
                #                                    filename, image, templates_z, scores, label,0,colors[boxNr%len(colors)],0,refreshRate-1, finalImages, 0)
                #j.positions = np.concatenate((j.positions,bboxes),0)
                j.padafter(refreshRate)
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
                bboxes, speed, finalImages = tracker(graph1, scfg1, hp, run, design, frame_name_list[i:i+refreshRate-1], pos_x, pos_y, target_w, target_h, final_score_sz,
                                                    filename, image, templates_z, scores, label,0,colors[boxNr%len(colors)],0,refreshRate-1, finalImages, 0)
                newBox = sfc_bbox(colors[boxNr%len(colors)],label,bboxes,0)
                newBox.padfront(i)
                Allbboxes.append(newBox)
                boxNr = boxNr +1
                print(bboxes)
            fname = i
            probs = [0]*len(Allbboxes)
            #print(Allbboxes)
            if (liveFeed):
                try:
                    oldFolderPath = folderPath.split('_')
                    oldFolderPath[2] = str(int(oldFolderPath[2]) - 1)
                    underscore = '_'
                    oldFolderPath = underscore.join(oldFolderPath)
                except:
                    print("Could not load old probabilies")
                    oldFolderPath = ""
            else:
                oldFolderPath = ""
            if(os.path.isfile(oldFolderPath+"/SiamfcBoxes.txt")):
                print('Extracting old probs')
                probs = getOldProbs(oldFolderPath, Allbboxes, [(item.split(','))[0] for item in boxes])
                print(probs)
            for j in range(len(finalImages)):
                #print(probs)
                angle = int(nucAngles.readline())
                calcProbs(finalImages[j],angle,Allbboxes,i+j, f)
                cv2.circle(finalImages[j],(int(float(1.0-angle/180.0)*testWidth),int(testHeight/2)),10,(0,0,225),-1)
                SiamfcVid.write(finalImages[j])
                cv2.imwrite(frame_name_list[j][0:-4]+'_siamfc.png',finalImages[j])
                fname = fname + 1
            #break
    f.close()
    SiamfcVid.release()
    return

def calcProbs(img, sourceAngle, allBoxes, imgNum, textFile):
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
        #cv2.line(img,(int(width/2+xcord),0),(int(width/2+xcord),height-1),(255,0,255),2)

    probsSum = 0
    for i in range(0,len(allBoxes)):
        thisBB = allBoxes[i].positions[imgNum]
        std1.append(0)
        std2.append(0)
        if(np.all(thisBB == -1)):
            continue
        position = int((thisBB[0]+thisBB[2]/2)/(width/12))
        probsSum = probsSum + allBoxes[i].prob
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
        if(np.all(allBoxes[i].positions == -1)):
            continue
        if(allstd2 > 0 and allstd1 > 0):
            allBoxes[i].prob = allBoxes[i].prob+std1[i]*.682/allstd1+std2[i]*.318/allstd2

        cv2.rectangle(img,(int(allBoxes[i].positions[imgNum][0]+allBoxes[i].positions[imgNum][2]/2),int(allBoxes[i].positions[imgNum][1]+allBoxes[i].positions[imgNum][3]/2+20)), 
        (int(allBoxes[i].positions[imgNum][0]+allBoxes[i].positions[imgNum][2]/2+100),int(allBoxes[i].positions[imgNum][1]+allBoxes[i].positions[imgNum][3]/2-30)),(255,255,255),-1)
        cv2.putText(img,'%f'%(allBoxes[i].prob/probsSum),(int(allBoxes[i].positions[imgNum][0]+allBoxes[i].positions[imgNum][2]/2),int(allBoxes[i].positions[imgNum][1]+allBoxes[i].positions[imgNum][3]/2)),cv2.FONT_HERSHEY_SIMPLEX,.5,(0,0,0))
        if i != 0:
            lineToWrite = lineToWrite + '|'
        lineToWrite  = lineToWrite+allBoxes[i].label+':'+str(allBoxes[i].prob)+':'+str(allBoxes[i].positions[imgNum][0])+','+str(allBoxes[i].positions[imgNum][1])+','+str(allBoxes[i].positions[imgNum][2])+','+str(allBoxes[i].positions[imgNum][3])
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

def main():

    #Command Line Arguments
    parser = argparse.ArgumentParser(description="Run WIND Project")
    parser.add_argument('-c','--clear',action='store_true', default = False, help= 'Delete old camera data')
    parser.add_argument('-ny','--noYolo',action='store_true', default = False, help = 'Do not use Yolo')
    parser.add_argument('-ns','--noSiamfc',action='store_true',default = False, help = 'Do not use Simafc')
    parser.add_argument('-nv','--noVideo', action='store_true',default = False, help = 'Do not generate video')
    parser.add_argument('-dl','--dataLocation', choices=['fromFile', 'fromCamera'], help = 'Use live camera or folder of images')
    parser.add_argument('-cn','--cameraNumber', default = 0, help = 'Camera number to use')
    parser.add_argument('-ff','--filesFolder',default = "")
    parser.add_argument('-rf','--refreshRate',default = 10, help= 'Refresh rate for siamfc')
    parser.add_argument('-t','--timeRecording',default = 3, help = 'Seconds to record from camera')
    
    #Assign command line arguments to global variables
    global cameraNumber
    global doYolo
    global doSiamfc
    global genVideos
    global refreshRate
    global liveFeed
    args = parser.parse_args()
    cameraNumber = int(args.cameraNumber)
    doYolo = not args.noYolo
    doSiamfc = not args.noSiamfc
    genVideos = not args.noVideo
    refreshRate = int(args.refreshRate)
    liveFeed = args.dataLocation == 'fromCamera'
    VideoLength = int(args.timeRecording) 

    #Command line argument error checking
    #Clearing out old data in the cameradata folder
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

    #Checking if file folder is valid
    if(args.dataLocation == 'fromFile'):
        if(os.path.isdir(args.filesFolder)):
            dirList = [os.path.join(args.filesFolder,d) for d in os.listdir(args.filesFolder) if os.path.isdir(os.path.join(args.filesFolder,d))]
            dirList.sort()
            testImage = Image.open(os.path.join(dirList[0],os.listdir(dirList[0])[0]))
            testWidth, testHeight  = testImage.size
        else:
            if(args.filesFolder == ""):
                print("No folder locatoin was given")
            else:
                print(args.filesFolder+" is not a valid file location")
            return
    #Check if the camera is valid
    else:
        try:
            cam = cv2.VideoCapture(cameraNumber)
            ret, testImage = cam.read()
            testHeight, testWidth = testImage.shape[:2]
            cam.release()
        except:
            print("Camera number given is not valid or connected")
            return
    
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
    
    fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    finalImages = []
    plt.xticks([]),plt.yticks([])
    YoloVid = cv2.VideoWriter('Yolov3Vid.avi',fourcc,10,(testWidth,testHeight))
    SiamfcVid = cv2.VideoWriter('SiamfcVid.avi',fourcc,10,(testWidth,testHeight))
    i = 0
    notDone = True

    now = datetime.datetime.now()
    if(args.dataLocation == 'fromCamera'):
        haha = threading.Thread(target=getImages,args=('CameraData/',10,VideoLength,database,now, graph, scfg))
        haha.start()
        haha.join()
        frame_name_list = _init_video('CameraData/%d_%d_%d/'%(now.hour,now.minute,now.second))
        while notDone and i < VideoLength and datetime.datetime.now()<now+datetime.timedelta(seconds=20+VideoLength) and genVideos:
            try:
                if os.path.isdir(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i))):
                    showYoloResult(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i)),YoloVid, False)
                    showSiamFCResult(os.path.join('CameraData','%d_%d_%d'% (now.hour, now.minute, now.second+i)),SiamfcVid, True)
                    i = i+1
                else:
                    time.sleep(.2)
            except(KeyboardInterrupt, SystemExit):
                notDone = False
    else:
        for dirName in dirList:
            if(doYolo):
                runYolo(dirName, database, fourcc, testWidth, testHeight, graph, scfg)
                if(genVideos):
                    showYoloResult(dirName, YoloVid, False)
            if(doSiamfc and dirName == dirList[0]):
                runSiamfc(dirName, fourcc, testWidth, testHeight, graph, scfg)
                if(genVideos):
                    showSiamFCResult(dirName, SiamfcVid, True)

    #haha.join()
    YoloVid.release()
    SiamfcVid.release()
    return


def _init_video(folderPath):
    video_folder = folderPath
    frame_name_list = [f for f in os.listdir(video_folder) if (f.endswith(".jpeg") or f.endswith(".png") and not('yolo' in f) and not('siamfc' in f))]
    frame_name_list = [os.path.join(video_folder, '') + s for s in frame_name_list]
    frame_name_list.sort()

    return frame_name_list


if __name__ == '__main__':
    sys.exit(main())
