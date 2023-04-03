class Point:
    """
    A high-level construct to manipulate and debug 3D points easily.
    """

    def __init__(self, arr=[0, 0, 0], x=None, y=None, z=None, interest_level=0, rest_time=1.0):
        """
        Default constructor.

        Args:
            arr: Array of 3D coordinates, x, y, and z respectively.
            x: the x coordinate, which overrides arr[0]
            y: the y coordinate, which overrides arr[1]
            z: the z coordinate, which overrides arr[2]
            interest_level: Optional interest level.
            rest_time: time to rest over each interest point with interest=0, in seconds.
        """
        self.x = arr[0] if x is None else x
        self.y = arr[1] if y is None else y
        self.z = arr[2] if z is None else z
        self.interest = interest_level
        self.rest_time = rest_time

    def toJSON(self):
        v = {
            'position': [self.x, self.y, self.z],
            'interest': self.interest
        }

        if self.interest == 0:
            v['restTime'] = self.rest_time

        return v

    def __repr__(self):
        """
        Debug information contained in this structure.

        Returns:
            A construct that summarizes the properties in space of this
            Point instance.
        """
        return f'Point(x:{self.x}; y:{self.y}; z:{self.z}; int:{self.interest})'
