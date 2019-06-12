import cv2
import datetime
import os

def getImages(folder_name,hour,minute,second,imagesPerSec,totalTime):
	cam = cv2.VideoCapture(0)
	
	for timeStep in range(0,totalTime):
		new_dir = os.path.join(folder_name,'%d_%d_%d'% (now.hour, now.minute, now.second))
		print(new_dir)
		if not os.path.exists(new_dir):
			os.makedirs(new_dir)
		
		f = open(new_dir+"groundtruth.txt","w+")
		for i in range(0,imagesPerSec):
		    ret, frame = cam.read()
		    #cv2.imshow("test", frame)
		    if not ret:
		        break
		    k = cv2.waitKey(1000/imagesPerSec)
		    img_name = folder_name+"opencv_frame_%03d.png" % i
		    cv2.imwrite(img_name, frame)
		    #print("{} written!".format(img_name))
		    f.write(("100 100 100 100\n"))

		f.close()
		second = second+1
	cam.release()
	#cv2.destroyWindow("test")

if __name__ == '__main__':
	now = datetime.datetime.now()
	getImages('CameraData/%d_%d_%d/'% (now.hour, now.minute, now.second))