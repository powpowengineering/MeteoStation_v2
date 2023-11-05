import requests
import json
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.dates
from datetime import datetime

def get_temperature(meteodata):
    temperature=[]
    for data in meteodata['feeds']:
        if data['field1'] is not None:
            temperature.append(data)
    return temperature

def get_meteo_data():
    response = requests.get('https://api.thingspeak.com/channels/1851639/fields/1.json?start=2023-09-01%2010:00:00&end=2023-09-25%2010:00:00')
    print(response.ok)
    # json_response = response.json()
    # print(json_response.keys())
    # print(json_response['channel'])
    # for data in json_response['feeds']:
    #     if data['field1'] is not None:
    #         print(data)
    return response



def main():

    # with open('temperature.json', 'w') as file:
    #     json.dump(get_temperature(get_meteo_data().json()), file)

    with open('temperature.json', 'r') as fp:
        data_temperature = json.load(fp)

    x_values=[]
    y_values=[]
    for data_dict in data_temperature:
        x_values.append(datetime.fromisoformat(data_dict['created_at'][:-1]))
        y_values.append(float(data_dict['field1']))
    # print(x_values)
    # print(y_values)
    dates = matplotlib.dates.date2num(x_values)
    plt.plot_date(dates, y_values)
    plt.show()





if __name__ == '__main__':
    main()