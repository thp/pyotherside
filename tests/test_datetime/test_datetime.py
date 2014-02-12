import datetime

def submit_datetime(dt):
    assert dt is not None
    print('Python got datetime:', dt)
    return dt

def submit_date(date):
    assert date is not None
    print('Python got date:', date)
    return date

def submit_time(time):
    assert time is not None
    print('Python got time:', time)
    return time

def get_datetime_value():
    v = datetime.datetime.now()
    print('Python returning datetime:', v)
    return v

def get_time_value():
    v = datetime.datetime.now().time()
    print('Python returning time:', v)
    return v

def get_date_value():
    v = datetime.datetime.now().date()
    print('Python returning date:', v)
    return v
