"""

"""

import scipy.io
import numpy as np
import matplotlib
# matplotlib.use('Agg')
import matplotlib.pyplot as plt

def format_number(x):
    return 255 - x.reshape((28,28))

def main():
    # data_read = scipy.io.loadmat("mnist_train.mat")
    # digits,labels = np.array(data_read['digits']),np.array(data_read['labels'])
    digits = np.fromfile("mnist.dat",dtype=np.uint8)
    N = len(digits) // 784
    digits = digits.reshape((N,784))
    print(digits.shape)

    # code to show a digit
    # plt.imshow(format_number(digits[0,:]),cmap='Greys')
    # plt.show()
    digits = digits.T
    
    N = 10 ** 4
    k = 20

    digits = digits[:,:N]
    num_features,_ = digits.shape

    # generates an array for 1xN rands from 0...1 and cast to integers from 0 to k-1
    group = np.zeros((N))
    for i in range(N):
        group[i] = i % k
    group = group.astype(np.int8)
    Z = np.zeros((num_features,k))
    D = np.zeros((N))
    prev_j = 0


    for epoch in range(100):
        for j in range(k):
            I = np.array([digits[:,d] for d in range(N) if group[d] == j]).T
            Z[:,j] = np.mean(I,axis=1)

        D = np.Infinity - np.zeros((N))
        for i in range(N):
            for j in range(k):
                this_d = sum((digits[:,i] - Z[:,j]) ** 2)
                if this_d < D[i]:
                    D[i] = this_d
                    group[i] = j

        Jscore = (1/N) * sum(D)
        print(Jscore)
        if abs(Jscore - prev_j) < 10 ** -6 * Jscore :
            print("ran for",epoch,"epochs")
            break
        else:
            prev_j = Jscore
        print("finished epoch",epoch)

    # display groups
    for i in range(k):
        plt.subplot(4,5,i+1)
        plt.imshow(format_number(Z[:,i]),cmap="Greys")
        plt.xticks(())
        plt.yticks(())
    
    plt.savefig("groups.png")



if __name__ == '__main__':
    main()

