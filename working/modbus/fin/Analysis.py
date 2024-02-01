import pymysql
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import itertools
import signal
import time
from itertools import count
from matplotlib.animation import FuncAnimation

HOST = "192.168.102.201"
USER = "vboxuser"
PASSWORD = "happy7"

# charset='utf8': 한글처리 (charset = 'utf8')
# autocommit=True: 결과 DB 반영 (Insert or update)
# cursorclass=pymysql.cursors.DictCursor: DB조회시 컬럼명을 동시에 보여줌
con = pymysql.connect(host = HOST, user = USER, password = PASSWORD, db = 'testdb', charset='utf8', autocommit=True, cursorclass=pymysql.cursors.DictCursor)
cur = con.cursor()

def handler(signum, frame):
    print("Ctrl+C 신호를 수신했습니다.")
    exit(0)

signal.signal(signal.SIGINT, handler)
data_count = 0
 
plt.style.use('fivethirtyeight')
 
cnt = 0
x_val = []
y_val = []
sample_x = []
sample_y = []

index = count()

idx = 1

def animate(i):
    global idx

    sql = "SELECT * FROM register WHERE IDX=%d" %idx
    cur.execute(sql)
    rows = cur.fetchall()

    df = pd.DataFrame(rows)
    #df.index=df.index+1
    df = df.drop(['DATE', 'IDX'], axis = 1)
    df = df.replace(0, np.NaN)
    print(rows)

    arr = []
    global cnt
    global x_val
    global y_val
    global sample_x
    global sample_y
    global data_count
    cnt+=1
    data_count+=1

    for i in range(10):
        arr.append(next(index))

    x_val.append(arr)
    y_val.append(list(df.loc[0]))

    sample_x = np.array(list(itertools.chain(*x_val)))
    sample_y = np.array(list(itertools.chain(*y_val)))

    plt.cla()
    plt.title('atmospheric pressure')
    plt.xlabel('Register')
    plt.ylabel('Value')
    plt.plot(sample_x, sample_y, marker = 'o')

    H_outlier = 60
    L_outlier = 40

    H_above = sample_y >= H_outlier
    L_below = sample_y <= L_outlier

    plt.scatter(sample_x[H_above], sample_y[H_above], s=100, c='r')
    plt.scatter(sample_x[L_below], sample_y[L_below], s=100, c='r')

    idx+=1

ani = FuncAnimation(plt.gcf(), animate, interval = 1000)
plt.tight_layout()
plt.show()