# Single View Modeling/Metrology

COMP5421 Project 3

## Usage

```
./svm.py [image file]
```

1) Define more than 2 parallel lines along each of x, y, z axis.
2) Define 3 reference points on the z=0 plane.
3) Define origin point.
4) Input lengths from each of the reference points to origin.
5) Compute everything and displays image warps.

## Note

* If image.json exists, simply run the program and press 'l' then 'Entr' to compute
* Number of axis points is unlimited but must be even
* Number of reference points must be 3
* Number of origin point must be 1
* Adjust scale of reference lengths if warped images are too small/big

## Keys for operations

### Scene definitions

```
'LClick'    #Puts point
'MClick'    #Pops point

'l'         #Loads predefined points and lengths
's'         #Saves currently defined points and lengths
        
'x'    	    #Define x-axis parallel lines
'y'    	    #Define y-axis parallel lines
'z'    	    #Define z-axis parallel lines
        
'r'    	    #Define reference points
'o'    	    #Define origin
        
'd'    	    #Prompts to input lengths of refernce points in CMD
'Entr' 	    #Calculates and displays warped images
```

### Utils

```
'c'    	    #Clears all defined points
'bksp' 	    #Clears CMD screen (only in linux env)
'esc'  	    #Exits program
'spbr'	    #Highlights defined points
```
