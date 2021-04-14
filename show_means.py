import numpy as np
import matplotlib.pyplot as plt

def format_number(x):
    return 255 - x.reshape((28,28))

def main():
    means = np.genfromtxt("means.csv", delimiter=',',dtype=np.uint8)[:,:-1]
    print(means.shape)

    for i in range(20):
        plt.subplot(4,5,i+1)
        plt.imshow(format_number(means[i,:]),cmap="Greys")
        plt.xticks(())
        plt.yticks(())
    plt.show()

if __name__ == '__main__':
    main()