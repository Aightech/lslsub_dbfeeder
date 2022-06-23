import psycopg2
from psycopg2 import Error
import numpy as np
from scipy.io import savemat
import sys

try:
    # Connect to an existing database
    connection = psycopg2.connect(user="lsldb_user",
                                  password="azerty",
                                  host="129.31.155.142",
                                  port="5432",
                                  database="lsldb")

    # Create a cursor to perform database operations
    cursor = connection.cursor()
    
    table_name = sys.argv[1]
    uid = sys.argv[2]
    filename = "data_"+table_name+"_"+uid;
    
    # Executing a SQL query
    cursor.execute("select time from "+table_name+" where uid='"+uid+"'")
    ind = cursor.fetchall();
    cursor.execute("select t from "+table_name+" where uid='"+uid+"'")
    time = cursor.fetchall();
    cursor.execute("select data from "+table_name+" where uid='"+uid+"'")
    values = cursor.fetchall();
    val = np.asarray(values)
    
    
    data = {'counter':ind,
            'time':time,
            'values':val[:,0]}
    savemat(filename+'.mat', data)
    

except (Exception, Error) as error:
    print( error)
finally:
    if (connection):
        cursor.close()
        connection.close()
