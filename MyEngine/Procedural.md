# Procedural generation

The purpose of this lesson is to navigate through different procedural algorithms in order to generate terrains, textures, map...

1. [Threshold 0 - Lesson](#threshold-0)
2. [Threshold 1 - Manager](#threshold-1)
3. [Threshold 2 - Drunkard Walk](#threshold-2)
4. [Threshold 3 - Cellular Automata](#threshold-3)
5. [Threshold 4 - Perlin Noise](#threshold-4)
6. [Threshold 5 - Wave Function Collapse](#threshold-5)

## Threshold 0

Lesson in class.

## Threshold 1

The purpose of this threshold is to create a `RandomMgr` which will handle `RandomInstance` to generate thread safe and determinitic randon sequences.

### Step 0

Let's start simple (and voluntary wrong). Create a `RandomMgr` in the same way we used to do it (and it to the Globals class...) such as
```cpp
class RandomMgr
{
public:
    void Init();
    void Shut();

    unsigned RandUInt(unsigned min, unsigned max);
    int RandInt32(int min, int max);
    double RandDouble(double min, double max);
    double RandNormalDouble(double center, double disp);

    void SetSeed(uint32_t seed) ;
    uint32_t GetSeed() const;

    static uint32_t GenerateRandomSeed();

private:
    std::default_random_engine RandomEng;
    uint32_t Seed;
};
```

Here a few infornation that might help you implementing this class:
- Take a look at [cpp reference](https://en.cppreference.com/w/cpp/numeric/random.html) (or any other documentation) to help you implementing the `RandXXX` function.
- Have a look at `std::random_device`

### Step 1

Integrate your manager in a debug window to test the different behaviours:
- Generate a random seed (should change any time you need it and the sequence must be different each time you restart your program).
- Test `RandXXX` functions. Those functions depend on the Seed you used. A same seed must always produces the same sequence.

### Step 2

We now have a manager that can be used from anywhere in our game at any time, which might be a good problem if we want our system to be determinitic. What if I need a certain seed for an generation algotithm and another one for the AI of our enemies? What if we use multi thread (we will see that later) and because of that, we lose control over the instruction order... To do so, we have to isolate each random process into different `RandomInstance`.

Create a `RandomInstance` that will contain : 
```cpp
class RandomInstance
{
public:
    unsigned RandUInt(unsigned min, unsigned max);
    int RandInt32(int min, int max);
    double RandDouble(double min, double max);
    double RandNormalDouble(double center, double disp);

    void SetSeed(uint32_t seed);
    uint32_t GetSeed() const { return Seed; }

private:
    std::default_random_engine RandomEng;
    uint32_t Seed;
};
```

*On your cpp side, you might need to rename the class name and voila, you should not need to update any code.

Modify the `RandomMgr` class to only be a wrapper around instances. 
```cpp
class RandomMgr
{
public:
    using InstanceId = unsigned;

    void Init();
    void Shut();

    InstanceId CreateInstance();
    void DestroyInstance(InstanceId instanceId);
    RandomInstance* GetInstance(InstanceId instanceId) const;

    static uint32_t GenerateRandomSeed();
};
```

Use the right container to store your instances and limite the number of allocation you need (to be clear, you should not need to allocate anything here).

Finally, modify your debug window to make it work and put in evidence the instance system.


## Threshold 2

The purpose of this threshold is to start simple and to implement another entry level algorithm : The drunkard walk.

The Drunkard walk algorithm is very straight forward. It consists of moving random walkers across a grid, marked each cell they walked on and chosse a random direction for each step.
1. Pick a random cell on the grid
2. Put a walker on this cell. Your walker must walk n steps and for each step, his direction is chosen randomly. Mark each cell he has walked on.
3. Pick a cell that has been marked and redo the same thing with another walker

### Step 0

Create a `DrunkardWalk` class containing at least the following members and methods. Each of those data will be explained at the right time. If you need to add any others members or methods, feel free to do it as you please. 

```cpp
class DrunkardWalk
{
public:
    DrunkardWalk(unsigned sizeX, unsigned sizeY, unsigned iteration, unsigned distance, unsigned spawnNumber);

    bool Generate();
    void GenerateTexture(sf::Texture& texture) const;

private:
    
    unsigned SizeX;
    unsigned SizeY;
    unsigned Iteration;
    unsigned Distance;
    unsigned SpawnNumber;
};
```

Use those default values to test your generation:
- SixeX = 256
- SixeY = 256
- Iteration = 5
- Distance = 25
- SpawnNuber = 1

### Step 1

As we made for others threshold, integrate the Drunkard Walk generation system into a ImGui window, so we can create multiple generations ith different parameters...

### Step 2

Implement the `Generate` and the `GenerateTexture` based on the algorithm described above. Here is a a small description of the attribute you need to use:
- SizeX, SizeY : size of the grid/image
- Iteration : number of walker that must be spwaned
- Distance : number of step that a single walker must do

About the `GenerateTexture`, do something simple by using a black and white coloration.

### Step 3

At this stage, your algorithm should create a big continuous mass. It might be intereting to create multiple mass across the grid. Use the SpawnNumber attribute to run the same algorithm on the same map at another spawn point (which has never been walked on).

### Going further

Add constraints such as cell that cannot be crossed for instance.

## Threshold 3

The purpose of this threshold is to implement a Cellular Automata algorithm to generate terrains or cave.

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off. The evolution of the cell's state depends on a specific ruleset

The most famous Cellular Automaton is the [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life).

Cellular Automata has plenty of useful properties that we can use. 

### Step 0

Create a `CellularAutomata` class containing at least the following members and methods. Each of those data will be explained at the right time. If you need to add any others members or methods, feel free to do it as you please. 

```cpp
class CellularAutomata
{
public:
    CellularAutomata(unsigned sizeX, unsigned sizeY, double spawnPercent, unsigned threshold, unsigned iteration);

    bool Generate();
    void GenerateTexture(sf::Texture& textures) const;

private:

    unsigned SizeX;
    unsigned SizeY;
    double SpawnPercent;
    unsigned NeightborThreshold;
    unsigned Iteration;
};
```

Use those default values to test your generation:
- SixeX = 256
- SixeY = 256
- SpawnPercent = 0.6
- NeightborThreshold = 5
- Iteration = 5

### Step 1

As we made for others threshold, integrate the Drunkard Walk generation system into a ImGui window, so we can create multiple generations ith different parameters...

### Step 2

Here are the ruleset that will defined our `CellularAutomata`:
- A cell cannot die. If it was set to on at some point in the process, it will survive no matter whata
- Borders count as a living cell
- If a cell is surronded by at least *NeightborThreshold* living cells (8 adjacent cells), this cell will turn living as well

Start to initialize your grid by setting all cell to living/dead according to the `SpawnPercent` attribute. Then, run `Iteration` loops following the ruleset above to propagate the living state.

### Step 3

Implement the `GenerateTexture` function, do something simple by using a black and white coloration.

### Going further

- Save the different iteration into multiple textures to the evolution of your algorithm result
- Use a non linear random to initialize your grid

## Threshold 4

The purpose of this threshold is to create a 2D Perlin noise generator. Such as the previous threshold, this generator must be deterministic and customizable.

### Step 0

Create a Perlin class containing at least the following members and methods. Each of those data will be explained at the right time. If you need to add any others members or methods, feel free to do it as you please. 

```cpp
class Perlin
{
public:
    Perlin(unsigned sizeX, unsigned sizeY, unsigned cellSize, unsigned octaves);

    bool Generate();
    void GenerateTexture(sf::Texture& texture) const;

private:
    
    unsigned SizeX;
    unsigned SizeY;
    unsigned CellSize;
    unsigned Octaves;
};
```

Use those default values to test your generation:
- SixeX = 256
- SixeY = 256
- CellSize = 16
- Octaves = 4

### Step 1

As we made for others threshold, integrate the Perlin generation system into a ImGui window, so we can create multiple generations ith different parameters...

### Step 2

It might be a good idea to implement the `GenerateTexture` function at this stage, because it will help you a lot debugging your progam. 

Each cell of your image will have a float value between -1 and 1. So, for each pixel, make a linear distribution between -1 (black) and 1 (white).

### Step 3

The first thing we need to do generate a Perlin noise is to create a grid over our final image.
- The final image's size will be **SizeX** and **SizeY**
- Each cell is a square of length **CellSize**

It might be a good idea to store the value of the grid size in new members (such as **GridSizeX** and **GridSizeY**), but it is up to you. 

For each intersection, so for each point of the grid, we will generate a random normalized 2D vector and store those vectors in the class because we will need it later. To help you visualizing what it is expected to validate this step, take a look at the following image.

![img](https://upload.wikimedia.org/wikipedia/commons/thumb/3/37/PerlinNoiseGradientGrid.svg/2560px-PerlinNoiseGradientGrid.svg.png)

### Step 4

**During this step, lots of type conversion (implicit and explicit). There are mandatory so, you must know how they work and understand them. Otherwise, it might be complicated**

Here is the step where the magic happen. The first thing you need to do is to iterate over each pixel and get the "Perlin" value for each pixel by using a function like
```cpp
float GetPerlinValue(float x, float y);
```

> *How is computed a Perlin value? Why do we use float instead of int/unsigned ?*
- Keep reading!

Let's use a random point as an example. 

![img](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTZgfh71kJJci3k2f_0EofiwmblnIDpd-81Uw&s)

For a given point, we will find the 4 corners that surronded this point and compute the 4 dot products between the vector formed by this point and a corner and the vector that we have generated earlier. Finally, we are going to interpolate those 4 values based on the proximity on the point with the corner. This interpolation will give us the Perlin value. To Summarize (and give more context) : 
1. Compute the dot product with the top left corner. This value will be called nx0.
2. Compute the dot product with the top right corner. This value will be called nx1.
3. Compute the interpolation between those two dot products based on the x axis. This value will be called ix0.
4. Compute the dot product with the bottom left corner. This value will be called ny0.
5. Compute the dot product with the bottom right corner. This value will be called ny1.
6. Compute the interpolation between those two dot products based on the x axis. This value will be called ix1.
7. Compute the interpolation between ix0 and ix1 based on the y axis. This value is the Perlin value.

You can use a classic interpolation algorithm if you want or use this one:
```cpp
float SmoothStep(float w)
{
    if (w <= 0.0)
    {
        return 0;
    }

    if (w >= 1.0)
    {
        return 1.0;
    }

    return w * w * (3.0 - 2.0 * w);
}

float Interpolate(float a0, float a1, float w) const
{
    return a0 + (a1 - a0) * SmoothStep(w);
}
```

By implementing the algorithm detailled above, you should have a very "blurry" Perlin noise.

### Step 5

To remove that blurry, it is now time to use the **Octaves** parameter which represent the number of point that which be taken into account to get the final Perlin Value. During Step 3, you took a single point (x, y) to compute the Perlin Value, so **Octaves** = 1. Two others parameters will also appear by using **Octaves** :
- *frequency* : multiplicative value that modifies the base position (x, y) after each octave. In step 3, frequency = 1.
- *amplitude* : multiplicative value that modifies the Perlin value got with a given position. In Step 3, amplitude = 1.

The final Perlin value is the sum of each individual Perlin Value * amplitude : 
PerlinValue = PerlinValue(x * freq1, y * freq1) * amp1 + PerlinValue(x * freq2, y * freq2) * amp2 + PerlinValue(x * freq3, y * freq3) * amp3 + ...

In our implementation:
- frequency = 1 and frequency *= 2 each octaves
- amplitude = 1 and amplitude /= 2 each octaves

### Step 6

Add a new way to render your texture.
Let's add a new `GenerateTexture` function that takes a vector of threshold/color (this vector should be customizable through an ImGui interface).

The purpose here is, by changing the rendering, generate a terrain map based on altitude. For instance let's use the following values:
- Perlin Value < -0.5 => dark blue
- Perlin Value < -0.1 => blue
- Perlin Value < 0 => light blue
- Perlin Value < 0.1 => yellow
- Perlin Value < 0.4 => green
- Perlin Value < 0.7 => brown
- Perlin Value > 0.7 => white

## Threshold 5

The purpose of this threshold is to implement Wave Function Collapse algorithm to generate procedural textures, tileset, map...

The WFC (Wave Function Collapse) is a rules driven algorithm. A set of elements to be placed (let's say tile for example) is given to the algorithm and a set of rules about how those elements interact with each others (A can be placed on the right of B but cannot be placed on the left of C). A grid is created and each cell can contain every element previously filled. The cell with the less **entropy** (the less number of possibilities) is **observed** (determined) randomly. This observation is **propagated** to the adjacent cells (because the next cell is determined and based on the ruleset previously filled, some possibilities might be lost). If possibilities have been lost that means this change mist be **propagated** to the next cells (like a wave)... Once the propagation is done, a new iteration is run (lowest entropy, observation, propagation) until an error occured (an impossible solution) or a stable solution (every cell has only one possibility).

Here is a teaser : https://www.youtube.com/watch?v=DOQTr2Xmlz0

### Step 0

Create a WaveFunctionCollapse class containing at least the following members and methods. Each of those data will be explained at the right time. If you need to add any others members or methods, feel free to do it as you please. 

```cpp
class WaveFunctionCollapse
{
public:

    enum class Direction
    {
        Right,
        Left,
        Up,
        Down
    }
    static const unsigned DirectionCount = 4;

    WaveFunctionCollapse(unsigned sizeX, unsigned sizeY);

    bool Generate();
    void GenerateTexture(sf::Texture& texture) const;

private:
    
    unsigned SizeX;
    unsigned SizeY;
};
```

### Step 1

As we made for others threshold, integrate the Perlin generation system into a ImGui window, so we can create multiple generations ith different parameters...

### Step 2

Let's start simple and manual with the following tiles (you are free to choose the color you want): 
- deep_sea (dark blue)
- sea (blue)
- sand (yellow)
- grass (green)
- mountain (brown)
- snow (white)

and the following rules:
- a tile can be placed next to another tile of the same type in all directions (up, down, right, left)
- "deep_sea" and "sea" can be placed next to each others in all directions
- "sea" and "sand" can be placed next to each others in all directions
- "sand" and "grass" can be placed next to each others in all directions
- "grass" and "mountain" can be placed next to each others in all directions
- "mountain" and "snow" can be placed next to each others in all directions

To do so:
1. Determine which way you are going to identify a specific pattern/tile for the rest of you progam. Create a new type that will be used in this function
```cpp
using WaveFunctionCollapseId = ...;
```

2. Add a function to register a new pattern. Here is an example of signature : 
```cpp
WaveFunctionCollapseId AddnewPattern(const std::string& name, const sf::Color& color);
```

You will need a container to store all data for a specific pattern. Choose it wisely

3. Add a function to add a new rule between two patterns. Here is an example of signature : 
```cpp
bool AddnewRule(const WaveFunctionCollapseId& patternA, const WaveFunctionCollapseId& patternB, Direction direction);
```

It might be a good idea to handle symmetry at this state (which which reduce the number of rule that must be prompted). You can also handle the reflect aspect (rule between A and B is the same for B and A).
I recommend you to have nested containers here (or nested structures) to handle the rule. One for the pattern, one for the direction and finally one for the rule.

### Step 3

At this stage, patterns and rules are set, so you can start the **Generate** function by initializing the final grid with all possibilities. Your **Generate** function should look like:
1. Initialize the Grid
2. Pick the cell with the lowest entropy
3. Observe it / Determine its type among its possibilities
4. Propagate the observation
5. Loop (from point 2) until the grid is stabilized

### Step 4

Let's start with the easiest part : Observation. A specific cell has a list of possibilities:
- if there are multiple possibilities, pick one randomly
- if there is no possibility, that means it is an impossible solution. So you have to block the algorithm
- if there is only one solution, that means that the cell is already stabilized. At this stage, you shouldn't observe a stabilized cell. That means that an error occured previously

### Step 5

Let's continue with another simple part : Picking the right cell.
We want to choose the next cell to observe. The right way to do that is to pick a cell that is not stabilized with the lowest entropy (with the lowest number of possibilities):
- if there is no possibility, that means it is an impossible solution. So you have to block the algorithm
- if there is only one solution, the cell is stabilized so skip it

There are multiple way to do that step:
- A dumb, simple but slow way : by looking at every cell each time
- A elegant, fast but more complex way : by using cache.

### Step 6

Let's finish with the complex part : Propagation

Once a cell is obsvered, you have to update the adjacent cells because based on the ruleset, some combinaisons might not be possibile anymore. In all directions, try to reduce the number of possibilities. If you succeed and reduce this number, this will have some consequences on this adjacent cells. So, you have to propagate the wave

### Step 7

Our algorithm is now finished and we have generated the all grid (or it fails so we have to rerun it). It is now time 
to implement the `GenerateTexture` function. For each cell on our grid, color the corresponding pixel using the right color, previously determined while filling the different patterns used.

## Threshold 6

We have created a WFC algorithm that is based on static rules which are described directly in the executable. It is now time to switch to a data driven approach. Multiple ways are possible such as providing a file containing the ruleset but we will move another approach.

The purpose of this threshold is to implement an algorithn to determine tile to use and rules to follow based on a set of images.

### Step 0

To make it simple and focus on the interestring part, let's add others functions to our `WaveFcuntionCollapse` class and modify it to handle thoses changes.

```cpp
bool LoadImage(unsigned patternSize, const std::filesystem::path& imgPath);
```

The first thing that this function must do is load the image in memory (it is not needed to create a texture, just the image is sufficient). Once this is done, we are going to split it in various patterns using the patternSize paremeter such as : 

![gif](https://blog.ptidej.net/content/images/2024/07/steps.gif)

in this example, patternSize = 2, and it will create this list of patterns (the rotation and reflection has been already been handled.)

![img](https://blog.ptidej.net/content/images/2024/07/cdf8b5d2-aa6a-41f1-9d9e-482e9ac06053.png)

Create a `Pattern` struct that will contain every pixels of a pattern in the right order and a `==` operator

```cpp
struct Pattern
{
    // Something to store the pixels

    bool operator==(const Pattern& other) const;
};
```

Store all unique patterns with their frequency (the number of time a pattern is found in the image) in your WFC class. It is way simpler to handle the rotation and reflection here. For each pixel you register, try to also register:
- the same pattern that you have rotated (0, 90, 180, 270)
- the symmetry on a `Pattern`

For both those case, be carefull with the mathematics you used for rotating or reflecting a `Pattern`. It might be a good idea to :
- Use very simple image at first (that you have creating yourself)
- Create a simple debug to visualize your work

You can find lots of image used for WFC algorithm [here](https://github.com/mxgmn/WaveFunctionCollapse).

### Step 1

Now, we have a complete list of `Pattern` to use, let's deduce the rules. To do it, nothing simpler. Try to match every `Pattern` you got with each others in every direction and check if the overlapping border is the same of not.

For example, Pattern A and B can be placed next to each other if the right border of A is the same (same pixels) than B's left border.

Using that compatibility system, you can create the list of rules that your algorithm must follow.

### Step 2

Modify the observe part of your algorithm to use the frequency of each possibilities instead of a uniform distribution between all possibilities.

### Step 3

Modify the `GenerateTexture` function to display each pattern instead of just one pixel.

## Threshold 7

We have a seen lots of algorithms to generate terrains, map... But the generation becomes nore powerful if we combine different algorithms. For instance
- Use the `CellularAutomata` to draw the shape of a cave and a `Perlin noise` to determine altitude variation.
- Use a `Perlin noise` to determine the temperature of a terrain and then, use a `WFC` to draw the world. Or the reversal, the `WFC` determines biomes and a `Perlin noise` complexifies the generation. 
- ...
