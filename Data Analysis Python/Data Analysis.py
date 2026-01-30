import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import statsmodels.api as sm
from sklearn.metrics import mean_squared_error

# Section 1, Data wrangling
# 1.1 a)

df_1 = pd.read_csv("Cycle_and_pedestrian data.csv", header=0)
df_1.head()

df_2 = pd.read_csv("Weather_data.csv", header=0)
df_2.head()

st_lucia_cyclist = df_1['Eleanor Schonell Bridge, St Lucia Cyclist']
st_lucia_pedestrian = df_1['Eleanor Schonell Bridge, St Lucia Pedestrian']
indooroopilly_cyclist = df_1['Jack Pesch Bridge, Indooroopilly Cyclist']
indooroopilly_pedestrian = df_1['Jack Pesch Bridge, Indooroopilly Pedestrian']

print(df_1.info)

plt.figure(figsize=(14,7))
plt.boxplot([st_lucia_cyclist, st_lucia_pedestrian, indooroopilly_cyclist, indooroopilly_pedestrian] , labels=['Eleanor Schonell Bridge Cyclist', 
            'Eleanor Schonell Bridge Pedestrian', 'Jack Pesch Bridge Cyclist', 'Jack Pesch Bridge Pedestrian'])
plt.title('Daily Usage Boxplot')
plt.ylabel('Count')
plt.grid(True)
plt.show()

# 1.1 b)

data1 = [st_lucia_cyclist, st_lucia_pedestrian, indooroopilly_cyclist, indooroopilly_pedestrian] 
data1_label = ['St Lucia Cyclist', 'St Lucia Pedestrian', 'Indooroopilly Cyclist', 'Indooroopilly Pedestrian']

df_1['Date'] = pd.to_datetime(df_1['Date'])

plt.figure(figsize=(14, 7))
for i in range(len(data1)):
    plt.plot(df_1.index, data1[i], label=data1_label[i])
plt.title('Total Bikeway Usage for Pedestrians and Cyclists')
plt.ylabel('Count')
plt.xlabel('Months')
plt.grid()
plt.legend()
plt.gca().xaxis.set_major_locator(mdates.MonthLocator())
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%b'))
plt.show()

# 1.1 c)
# Filter 1 for removing variation

def percent_dif(pedestrain, cyclist):
    val1 = pedestrain / (pedestrain + cyclist)
    val2 = 1 - val1
    percentages = [val1, val2]
    return percentages

for i in range(len(st_lucia_cyclist)):
    percentage = percent_dif(indooroopilly_pedestrian[i], indooroopilly_cyclist[i])
    if i > 232:
        total_count = st_lucia_cyclist[i] + st_lucia_pedestrian[i]
        st_lucia_pedestrian[i] = total_count * percentage[0] 
        st_lucia_cyclist[i] = total_count * percentage[1] 
        
# Filter 2 for removing decimal values

for i in range(len(st_lucia_cyclist)):
    st_lucia_cyclist[i] = int(st_lucia_cyclist[i])
    
# Returning the values back to the df_1

df_1['Eleanor Schonell Bridge, St Lucia Cyclist'] = st_lucia_cyclist
df_1['Eleanor Schonell Bridge, St Lucia Pedestrian'] = st_lucia_pedestrian

# Filter 3 for smoothing plot

filter_weekly = 7

st_lucia_cyclist_weekly = st_lucia_cyclist.rolling(window=filter_weekly, center=True).mean()
st_lucia_pedestrian_weekly = st_lucia_pedestrian.rolling(window=filter_weekly, center=True).mean()
indooroopilly_cyclist_weekly = indooroopilly_cyclist.rolling(window=filter_weekly, center=True).mean()
indooroopilly_pedestrian_weekly = indooroopilly_pedestrian.rolling(window=filter_weekly, center=True).mean()
weekly_data = [st_lucia_cyclist_weekly, st_lucia_pedestrian_weekly, indooroopilly_cyclist_weekly, indooroopilly_pedestrian_weekly]
weekly_date = []
for length in range(len(st_lucia_cyclist_weekly)):
    weekly_date.append(length)

plt.figure(figsize=(14, 7))
for i in range(len(data1)):
    plt.plot(weekly_date, weekly_data[i], label=data1_label[i])
plt.title('Total Bikeway Usage for Pedestrians and Cyclists Clean')
plt.ylabel('Count')
plt.xlabel('Months')
plt.legend()
plt.grid()
plt.gca().xaxis.set_major_locator(mdates.MonthLocator())
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%b'))
plt.show()


# 1.2 a)


rainfall = df_2['Rainfall amount (millimetres)'].fillna(df_2['Rainfall amount (millimetres)'].mean())
max_temp = df_2['Maximum temperature (Degree C)'].fillna(df_2['Maximum temperature (Degree C)'].mean())
solar = df_2['Daily global solar exposure (MJ/m*m)'].fillna(df_2['Daily global solar exposure (MJ/m*m)'].mean())
weather = [rainfall, max_temp, solar]
weather_labels = ['Rain Fall', 'Maximum Tempratures', 'Solar Exposure']
weather_titles_box = ['Rainfall Boxplot', 'Maximum Temprature Boxplot', 'Solar Exposure Boxplot']
weather_ylables = ['mm', 'Degrees C', 'MJ/m^m']


fig, axs = plt.subplots(1, 3, figsize=(14, 7))
for i in range(len(weather)):
    axs[i].boxplot(weather[i] , labels=[weather_labels[i]])
    axs[i].set_title(weather_titles_box[i])
    axs[i].set_ylabel(weather_ylables[i])
fig.suptitle('Weather Data')
plt.tight_layout()
fig.subplots_adjust(top=0.9)
plt.show()

# 1.2 b)

df_2['Date'] = pd.to_datetime(df_2['Date'])
weather_dates = df_2['Date']
weather_titles_linear = ['Rainfall Linegraph', 'Maximum Temprature Linegraph', 'Solar Exposure Linegraph']

for i in range(len(weather)):
    plt.figure(figsize=(14, 7))
    plt.plot(weather_dates, weather[i] , label=[weather_labels[i]])
    plt.title(weather_titles_linear[i])
    plt.ylabel(weather_ylables[i])
    plt.xlabel('Months')
    plt.grid()
    plt.gca().xaxis.set_major_locator(mdates.MonthLocator())
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%b'))
plt.show()

# 1.2 c)

# Filter 1 removing 95th percentile

for i in range(len(rainfall)):
    if rainfall[i] > np.percentile(rainfall, 95):
        weather[0][i] = 0
        
# Filter 2 Filling missing data with mean values and creating a data frame that holds the date and its corresponding value

weather_clean = []
for i in range(len(weather)):
    temp_df2 = pd.DataFrame({
        'Date': weather_dates,
        'Value': weather[i]
    })
    temp_df2['Value'] = temp_df2['Value'].fillna(temp_df2['Value'].mean())
    temp_df2['Month'] = temp_df2['Date'].dt.to_period('D')
    weather_clean.append(temp_df2)
        
# Re-adding the data back to df_2

df_2['Rainfall amount (millimetres)'] = weather_clean[0]['Value']
df_2['Maximum temperature (Degree C)'] = weather_clean[1]['Value']
df_2['Daily global solar exposure (MJ/m*m)'] = weather_clean[2]['Value']
        
# Filter 3 setting the data to weekly for eaiser visulisation of data

weather_week = []
for i in range(len(weather)):
    weather_week.append(weather[i].rolling(window=filter_weekly, center=True).mean())
weather_weekly_date = []
for i in range(len(weather_week[0])):
    weather_weekly_date.append(i)
        
# 1.2 d)

for i in range(len(weather)):
    plt.figure(figsize=(14, 7))
    plt.plot(weather_weekly_date, weather_week[i] , label=[weather_labels[i]])
    plt.title(weather_titles_linear[i] + " Filtered")
    plt.ylabel(weather_ylables[i])
    plt.xlabel('Months')
    plt.grid()
    plt.gca().xaxis.set_major_locator(mdates.MonthLocator())
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%b'))
plt.show()

# 2.1 a)

def sum_for_year(data):
    new_data = []
    summed_data = []
    for i in range(len(data)):
        series = data[i]
        mean_val = series.mean()
        sum_val = 0
        for j in range(51):
            value = series[j]
            if pd.isna(value):
                value = mean_val
            sum_val += value
        new_data.append(sum_val)
    summed_data.append(new_data[0] + new_data[1]) 
    summed_data.append(new_data[2] + new_data[3]) 
    return summed_data

yearly_totals = sum_for_year(weekly_data)
barplot_names = ['St Lucia', 'Indooroopilly']

plt.figure(figsize=(14, 7))
for i in range(len(yearly_totals)):
    plt.bar(barplot_names[i], yearly_totals[i])
plt.ylabel('Count')
plt.xlabel('Location')
plt.title('Yearly Total Bikeway Usage')
plt.tight_layout()
plt.show()

# 2.1 b)

print(df_1[['Date','Jack Pesch Bridge, Indooroopilly Cyclist']].describe())
print(df_1[['Date','Eleanor Schonell Bridge, St Lucia Cyclist']].describe())
print(df_1[['Date','Jack Pesch Bridge, Indooroopilly Pedestrian']].describe())
print(df_1[['Date','Eleanor Schonell Bridge, St Lucia Pedestrian']].describe())

df_1['Month'] = df_1['Date'].dt.to_period('M')
print(df_1.groupby('Month')['Jack Pesch Bridge, Indooroopilly Cyclist'].std().idxmax(),df_1.groupby('Month')['Jack Pesch Bridge, Indooroopilly Cyclist'].std().max())
print(df_1.groupby('Month')['Eleanor Schonell Bridge, St Lucia Cyclist'].std().idxmax(),df_1.groupby('Month')['Eleanor Schonell Bridge, St Lucia Cyclist'].std().max())
print(df_1.groupby('Month')['Jack Pesch Bridge, Indooroopilly Pedestrian'].std().idxmin(),df_1.groupby('Month')['Jack Pesch Bridge, Indooroopilly Pedestrian'].std().min())
print(df_1.groupby('Month')['Eleanor Schonell Bridge, St Lucia Pedestrian'].std().idxmin(),df_1.groupby('Month')['Eleanor Schonell Bridge, St Lucia Pedestrian'].std().min())

print(df_1[['Date','Jack Pesch Bridge, Indooroopilly Cyclist']].median())
print(df_1[['Date','Eleanor Schonell Bridge, St Lucia Cyclist']].median())
print(df_1[['Date','Jack Pesch Bridge, Indooroopilly Pedestrian']].median())
print(df_1[['Date','Eleanor Schonell Bridge, St Lucia Pedestrian']].median())

# 2.2 a)

fig, axs = plt.subplots(1, 3, figsize=(16, 5))
bins = 20

axs[0].hist(df_2['Maximum temperature (Degree C)'], bins=bins, edgecolor='black')
axs[0].set_title('Histogram of Max Temperature')
axs[0].set_xlabel('Temperature (Degrees C)')
axs[0].set_ylabel('Frequency')

axs[1].hist(df_2['Daily global solar exposure (MJ/m*m)'], bins=bins, edgecolor='black')
axs[1].set_title('Histogram of Solar Exposure')
axs[1].set_xlabel('MJ/m*m')

axs[2].hist(df_2['Rainfall amount (millimetres)'], bins=bins, edgecolor='black')
axs[2].set_title('Histogram of Rainfall')
axs[2].set_xlabel('Rainfall (mm)')

plt.tight_layout()
plt.show()

# 2.2 b)

# Calculating mean
for i in range(len(weather_clean)):
    print(weather_clean[i]['Value'].mean())

# Calculating median
for i in range(len(weather_clean)):
    print(weather_clean[i]['Value'].median())

# Calculating range
for i in range(len(weather_clean)):
    print(weather_clean[i]['Value'].max() - weather_clean[i]['Value'].min())

# Calculating standard deviation
monthly_std = []
for i in range(len(weather)):
    temp_df2 = pd.DataFrame({
        'Date': weather_dates,
        'Value': weather[i]
    })
    temp_df2['Value'] = temp_df2['Value'].fillna(temp_df2['Value'].mean())
    temp_df2['Month'] = temp_df2['Date'].dt.to_period('M')
    std_by_month = temp_df2.groupby('Month')['Value'].std()
    monthly_std.append(std_by_month)

# Standard deviation max from month
for i in range(len(monthly_std)):
    print(f"{monthly_std[i].max()} <----")
    print(monthly_std[i])

# Standard deviation min from month
for i in range(len(monthly_std)):
    print(f"{monthly_std[i].min()} <----")
    print(monthly_std[i])

# 3.1

correlation1 = np.corrcoef(df_1['Eleanor Schonell Bridge, St Lucia Cyclist'], df_1['Eleanor Schonell Bridge, St Lucia Pedestrian'])[0, 1]
print(f"Correlation coefficient between St Lucia Cyclist and St Lucia Pedestrian: {correlation1}")
correlation2 = np.corrcoef(df_1['Jack Pesch Bridge, Indooroopilly Cyclist'], df_1['Jack Pesch Bridge, Indooroopilly Pedestrian'])[0, 1]
print(f"Correlation coefficient between Indooroopilly Cyclist and Indooroopilly Pedestrian: {correlation2}")
correlation3 = np.corrcoef(df_1['Eleanor Schonell Bridge, St Lucia Cyclist'], df_1['Jack Pesch Bridge, Indooroopilly Cyclist'])[0, 1]
print(f"Correlation coefficient between St Lucia Cyclist and Indooroopilly Cyclist: {correlation3}")
correlation4 = np.corrcoef(df_1['Eleanor Schonell Bridge, St Lucia Pedestrian'], df_1['Jack Pesch Bridge, Indooroopilly Pedestrian'])[0, 1]
print(f"Correlation coefficient between St Lucia Pedestrian and Indooroopilly Pedestrian: {correlation4}")

# 3.2

plt.figure(figsize=(8, 6))
plt.scatter(df_1['Eleanor Schonell Bridge, St Lucia Pedestrian'], df_1['Eleanor Schonell Bridge, St Lucia Cyclist'], color='blue', marker='x')
plt.xlabel('Pedestrians')    
plt.ylabel('Cyclists')
plt.grid(True)
plt.title(f'Correlation Between St Lucia Pedestrians and Cyclists : {round(correlation1, 4)}')
plt.show()

# 4.1 a)

X1 = sm.add_constant(df_2[['Rainfall amount (millimetres)', 'Maximum temperature (Degree C)', 'Daily global solar exposure (MJ/m*m)']])
y1 = df_1['Jack Pesch Bridge, Indooroopilly Cyclist']

model1 = sm.OLS(y1, X1).fit()
print(model1.summary())

fitted_values1 = model1.fittedvalues
predictions1 = model1.get_prediction(X1)
conf_int1 = predictions1.conf_int()
lower_bounds1 = conf_int1[:, 0]
upper_bounds1 = conf_int1[:, 1]

for i, name in enumerate(X1.columns[1:]):
    x1 = X1[name]
    plt.figure(figsize=(8, 6))
    plt.scatter(x1, y1, label='Data', color='blue')
    
    X1_single = sm.add_constant(x1)
    model_single = sm.OLS(y1, X1_single).fit()
    y_pred = model_single.fittedvalues
    
    pred1 = model_single.get_prediction(X1_single)
    ci1 = pred1.conf_int()
    
    plt.plot(x1, y_pred, color='red', label='Fit')
    plt.fill_between(x1, ci1[:, 0], ci1[:, 1], color='gray', alpha=0.3, label='95% CI')
    plt.xlabel(name)
    plt.ylabel('Cyclist Count')
    plt.title(f'Cyclist Usage vs {name}')
    plt.legend()
    plt.grid(True)
plt.show()

# 4.1 b)

X2 = sm.add_constant(df_2[['Rainfall amount (millimetres)', 'Maximum temperature (Degree C)', 'Daily global solar exposure (MJ/m*m)']])
y2 = df_1['Jack Pesch Bridge, Indooroopilly Pedestrian']

model2 = sm.OLS(y2, X2).fit()
print(model2.summary())

fitted_values2 = model2.fittedvalues
predictions2 = model2.get_prediction(X2)
conf_int2 = predictions2.conf_int()
lower_bounds2 = conf_int2[:, 0]
upper_bounds2 = conf_int2[:, 1]

for i, name in enumerate(X2.columns[1:]):
    x2 = X2[name]
    plt.figure(figsize=(8, 6))
    plt.scatter(x2, y2, label='Data', color='blue')
    
    X2_single = sm.add_constant(x2)
    model_single = sm.OLS(y2, X2_single).fit()
    y_pred = model_single.fittedvalues
    
    pred2 = model_single.get_prediction(X2_single)
    ci2 = pred2.conf_int()
    
    plt.plot(x2, y_pred, color='red', label='Fit')
    plt.fill_between(x2, ci2[:, 0], ci2[:, 1], color='gray', alpha=0.3, label='95% CI')
    plt.xlabel(name)
    plt.ylabel('Cyclist Count')
    plt.title(f'Cyclist Usage vs {name}')
    plt.legend()
    plt.grid(True)
plt.show()