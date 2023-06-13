import psycopg2
from psycopg2 import Error
import numpy as np
from scipy.io import savemat
import sys

try:
    # Connect to an existing database
    connection = psycopg2.connect(user="lsldb_user",
                                  password="azerty",
                                  host="localhost",
                                  port="5432",
                                  database="lsldb")

    # Create a cursor to perform database operations
    cursor = connection.cursor()
    
    table_name = sys.argv[1]
    filename = "data_"+table_name;
    
    # Executing a SQL query
    cursor.execute("select index from "+table_name)
    ind = cursor.fetchall();
    cursor.execute("select time from "+table_name)
    time = cursor.fetchall();
    cursor.execute("select data from "+table_name)
    values = cursor.fetchall();
    val = np.asarray(values)
    
    
    data = {'index':ind,
            'time':time,
            'data':val[:,0]}
    savemat(filename+'.mat', data)
    

except (Exception, Error) as error:
    print( error)
finally:
    if (connection):
        cursor.close()
        connection.close()
