<?xml version="1.0" ?>
<!--Rigid Body file, created by Rigid Body Designer-->
<rigid_body name="indicator">
    <mesh filename="indicator.mesh" />
    <properties mass="150" inertia="27.5702 54.1003 77.5098" />
    <primitive name="box1" shape="box">
        <properties position="0 0.31 0" rotation="0 0 0" size="2 0.82 0.2" />
    </primitive>
    <primitive name="box2" shape="box">
        <properties position="0.5 -0.28 0" rotation="0 0 0" size="0.2 0.390001 0.13" />
    </primitive>
    <primitive name="box3" shape="box">
        <properties position="-0.5 -0.28 0" rotation="0 0 0" size="0.2 0.390001 0.12" />
    </primitive>
    <primitive name="hull_1" shape="convex_hull">
        <properties position="0 0 0" rotation="0 0 0">
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="-1 -0.681745 0.199782" />
            <vertex position="0.999756 -0.681745 0.199782" />
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="-1 -0.481743 0.0997935" />
            <vertex position="-1 -0.681745 0.199782" />
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="-1 -0.481743 -0.0999888" />
            <vertex position="-1 -0.481743 0.0997935" />
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="1 -0.481743 -0.0999888" />
            <vertex position="-1 -0.481743 -0.0999888" />
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="1 -0.681745 -0.199978" />
            <vertex position="1 -0.481743 -0.0999888" />
            <vertex position="1 -0.481743 0.0999888" />
            <vertex position="0.999756 -0.681745 0.199782" />
            <vertex position="1 -0.681745 -0.199978" />
            <vertex position="0.999756 -0.681745 0.199782" />
            <vertex position="-1 -0.681745 -0.199978" />
            <vertex position="1 -0.681745 -0.199978" />
            <vertex position="0.999756 -0.681745 0.199782" />
            <vertex position="-1 -0.681745 0.199782" />
            <vertex position="-1 -0.681745 -0.199978" />
            <vertex position="-1 -0.681745 0.199782" />
            <vertex position="-1 -0.481743 0.0997935" />
            <vertex position="-1 -0.681745 -0.199978" />
            <vertex position="-1 -0.481743 0.0997935" />
            <vertex position="-1 -0.481743 -0.0999888" />
            <vertex position="-1 -0.681745 -0.199978" />
            <vertex position="-1 -0.481743 -0.0999888" />
            <vertex position="1 -0.681745 -0.199978" />
            <vertex position="-1 -0.681745 -0.199978" />
            <vertex position="-1 -0.481743 -0.0999888" />
            <vertex position="1 -0.481743 -0.0999888" />
            <vertex position="1 -0.681745 -0.199978" />
        </properties>
    </primitive>
</rigid_body>
