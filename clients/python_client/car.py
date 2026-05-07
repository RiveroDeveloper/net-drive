class Car:
    def __init__(self, speed, temp, direction, battery):
        self.speed = speed
        self.battery = battery
        self.temp = temp
        self.direction = direction
        
    def __init__(self):
        self.speed = 0
        self.battery = 0
        self.temp = 0
        self.direction = "NONE"

    def showStateData(self):
        data = f"{self.speed},{self.temp},{self.direction},{self.battery}"
        return "DATA" + data
    
    def getStateData(self):
        return self.speed, self.temp, self.direction, self.battery
    
    def updateState(self, speed, temp, direction, battery):
        self.speed = speed
        self.battery = battery
        self.temp = temp
        self.direction = direction
        
car = Car()
        
        
        