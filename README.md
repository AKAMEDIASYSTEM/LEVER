# LEVER
####Experiments in electrovibration interaction strategies, inspired by Disney Research papers "Tesla Touch" and "Revel"

Here are the papers I am trying to implement here:
http://www.ivanpoupyrev.com/e-library/2010/teslatouchUIST2010.pdf

http://www.disneyresearch.com/wp-content/uploads/REVEL.pdf

###Summary
Basically, the papers describe a way to augment surfaces with _dynamically changeable tactile properties_ - a surface can change from sticky to slippery basically at will.

###Status
This work is incomplete and ongoing - for one thing, while the circuit decribed here works, I have yet to implement current-sensing feedback as described in the papers. I'm eager to accomplish this because it allows normalization of sensation across people, skin moistures, and time.

###Why I am interested
I'm interested in experimenting with this technology as applied to various notification strategies, particularly on mobile and wearable devices.
I would like to develop interactions in which a person queries a "stack" of stuff waiting for them, rather than being interrupted by a push notification.
The appeal of this technology in this application is that it's totally silent and able to be done without interrupting the person's conversation by stealing focus.
If this technology is implemented on the back of a mobile device, for example, simply rubbing a finger along the back of the phone is all that is needed to see the magnitude of my "stack."
Of course, "stack" can be many things: number of unread emails, number of to-do items, number of places of interest very near me, and so on.

###This repository
The purpose of this repo is to make my work shareable - I had a fair amount of trouble trying to go from the research paper to a safe, working circuit, and I'm hoping to save others some work.
The circuitboard isn't what I would put onto a phone - it is instead a platform for trying out different waveforms, intensities, and so on.
The intent is that you will use hte board to figure out the correct implementation details and then generate your own board with a more appropriate layout.

###Thanks
I'm so grateful to Disney Research, Ivan Poupyrev, Olivier Bau, and others who worked on the papers for making their work available - I never would have known aobut this cool phoneomenon without their shared work! Thanks as well to nytlabs for supporting my explorations and letting me share in kind.
