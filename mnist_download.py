import mnist
import os
import numpy as np
import matplotlib.pyplot as plt

def main():
    download_dir = os.path.join(os.curdir,"mnist")
    if not os.path.exists(download_dir):
        os.makedirs(download_dir)

    mnist.temporary_dir = lambda: download_dir
    print("downloading mnist data... (this may take a while)")
    train_images = mnist.train_images()
    print("done!")
    N,h,w = len(train_images),len(train_images[0]),len(train_images[0][0])
    data_np = np.array(train_images).reshape((N,h*w)).astype(np.uint8)
    

    with open("mnist.dat","wb") as data_out:
        data_out.write(data_np.tobytes(order='C'))

if __name__ == '__main__':
    main()