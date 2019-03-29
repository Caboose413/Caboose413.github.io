# Resume.

Hello, my name is Caboose I'm a 25 year old indie game developer from Germany.
I've been using the unreal engine since 2014, sofar I have been working on 3 different titels.

my skills range from 3D Modeling, Anmation, material programming to coding with BP/C++. 
one of the most important part for me is to keeping track of performance and optimization.

## The Facility
2014-2016

The Facility is a survival horror game playing shortly after the 2nd world war. you play as Nathan who has been searching for a hidden Nazi bunker
which is supposed to be full of treasure. but little did you know about the real reasons of why you were trying to find the bunker.

**Position:**
- Co-Lead Developer.
- 3D Modeling.
- Level Design.

## Bunny hop league
2016-2018

Bunny hop league is a fast paced parkour game which is making use of the famous bunny hopping and surfing technique from Counter-Strike.
by moving your mouse left and right you gain speed to finish the parkour first.

**Position:**
- Lead Developer.
- Level Design.
- Programming.
 
Bunny hop league is currently available on steam you can check it out here! [Link to Steam ](https://store.steampowered.com/app/429780/Bunny_Hop_League/)

![Image of Yaktocat](https://github.com/Caboose413/Caboose413.github.io/blob/master/assets/bunnyhopleague/BhlExample.gif?raw=true)

### key features:

- **Parkour:** a key aspect of bhl is to figure out and learn what the best route is in each parkour!
- **Multiplayer:** play against your friends and race for the fastest time!
- **Item trade:** trade various items which you can earn by finishing parkours!

## Celestial
2018-2019

My current project is called Celestial it is a Co-op survival adventure. you take place as a Physicist astronaut who is tasked with testing the first warp drive created by humankind.
but shortly after starting up the drive and beginning the test strange things start to happen the warp drive starts to overhead and an abyss looking like a black hole starts to emerge in front of your ship,
the next thing you remember is waking up above an unknown planet with no clue on what just happen or how to get back.

**Position:**
- Lead Developer.
- Level Design.
- 3D Modeling.
- Programming.

![Image of Yaktocat](https://github.com/Caboose413/Caboose413.github.io/blob/master/assets/Celestial/CelestialGif01.gif?raw=true)

### Content of this project:

- **Vast World:** Play on a Spherical world of 380km2 no borders or walls to stop you!
- **Survive:** Survive in a hostile envoirament against alien creatuers and machines.
- **Co-op:** Play and survive together with friends and explore a new world!


## Celestial Development

During the development of Celestial I faced many issues with making a character walk on a sphere,
or how to create a navigation system for AI without a Navmesh. 
In this section I will explain how I overcome those issues and give you some examples too!

**Walking on a Sphere**

Unreal engine already provides really usefull functions which will calculate a rotation for you based on vector inputs.
For example this code will align my capsule component Upwards depending it it's location on the sphere.

{% highlight c++ %} const FMatrix AlignRotation = FRotationMatrix::MakeFromZX(CapsuleComp->GetActorLocation, CapsuleComp->GetForwardVector()); {% endhighlight %}

this was one of the easier parts, the next issue was how to create responsive movement logic which includes jumping, falling and slope dedection. since the default unreal engine Walking "movement mode" can't be used here I had to abuse the flying "movement mode" and add some logic to it.


So I started off with using a sphere trace which always points down and then take the impact location and move it up based on our current upvector.

{% highlight c++ %}const FVector Location = Parent->GetActorLocation();
const FVector EndTrace = Location + (CapsuleComp->GetUpVector() * -1) * 5000.0f;
const bool isHit = GetWorld()->SweepSingleByChannel(OutHit, Location, EndTrace, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(6.0f));
FVector FloatingLocation = OutHit.Location + (CapsuleComp->GetUpVector() * Height);
Parent->SetActorLocationAndRotation(FloatingLocation, AlignRotation, true, &SweepWalkCheck, ETeleportType::None); {% endhighlight %}