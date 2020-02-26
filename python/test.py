import FeiRays

scene = FeiRays.Scene(800, 400)
scene.add_sunlight((1,1,1), 2, (500, 500, 500))

model = FeiRays.Transform()
model.translate(0,-0.2,0)
model.scale(6,0.2,6)
scene.add_colored_cube(model, color=(0.7,0.7,0.7))

model = FeiRays.Transform()
model.translate(0,1,-2)
model.rotate(45, (0,1,0))
scene.add_colored_cube(model, FeiRays.METAL, color=(0.8,0.6,0.2), fuzz=0.3)

model = FeiRays.Transform()
model.translate(4,1,-2)
model.rotate(45, (0,1,0))
scene.add_colored_cube(model, color=(0.1,0.2,0.5))

model = FeiRays.Transform()
model.translate(-4,1,-2)
model.rotate(45, (0,1,0))
scene.add_colored_cube(model, FeiRays.DIELECTRIC, ref_idx=1.5)

model = FeiRays.Transform()
model.translate(0,1,2)
scene.add_colored_sphere(model, FeiRays.DIELECTRIC, color=(0.5, 1.0, 0.7), ref_idx=1.5, density=10)

model = FeiRays.Transform()
model.translate(4,1,2)
scene.add_colored_sphere(model, FeiRays.DIELECTRIC, color=(1.0, 0.5, 0.7), ref_idx=1.5, density=10)

model = FeiRays.Transform()
model.translate(-4,1,2)
scene.add_colored_sphere(model, FeiRays.DIELECTRIC, color=(0.7, 0.5, 1.0), ref_idx=1.5, density=10)

scene.set_camera((0,8,8),(0,0,0),(0,1,0), 45)
scene.trace(1000, 200)

img=scene.get_image()
img.save("test.png")

