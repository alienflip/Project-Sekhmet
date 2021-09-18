# Sekhmet

#### idea:::

We have an array of data: it represents the location of a pixel with some other data attached:

Each pixel therefore has a position (x, y).

Additionally, we have a velocity attached to each data-point. This represents whether or not the pixel will transfer state to another pixel in it's vicinity.

This means that an array of 4 by 4 pixels looks like:

(....)(....)(....)(....)

(....)(....)(....)(....)

(....)(....)(....)(....)

(....)(....)(....)(....)

In the program.

And so each row of an array actually represents rowWidth / 4 pixels in reality, since the we will be working modulo 4 as pixels need 4 data points to represent themselves.

Using openCL, we can take advantage of massive parrellelism in order to do boid calculations (https://en.wikipedia.org/wiki/Boids) on local pixels, and then use global space to keep track of the averagees of the whole group.

####:::
