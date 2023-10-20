
import unittest

import numpy as np
import matplotlib.pyplot as plt

from python import dataset
from build import config

class Datataset(unittest.TestCase):

    def test_bore_sight(self):
        generator = dataset.DatasetGenerator()

        frequency = 8e3

        expected_generator = dataset.sine_wave_samples_generator(1.0, frequency, config.SAMPLE_RATE)


        # Add dummy source 
        generator.add_source(0, 0, frequency)

        # Populate the pipeline
        for i in range(config.N_FRAMES):
            generator.add_beams()
            expected_frame = next(expected_generator)

        beams = generator.get_last_frame()

        #plt.plot(expected_frame, label="Expected")
        #plt.plot(beams[0], label="Result")
        #plt.legend()
        #plt.show()

        for i in range(beams.shape[0]):
            same = np.allclose(beams[i], expected_frame, rtol=1e-05, atol=1e-08)

            self.assertTrue(same)


            

    def test_mimo_generation2(self):
        """Testing that generating a mimo works as expected"""
        
        #return
        generator = dataset.DatasetGenerator()

        # Add some sources
        # generator.add_source(50, 20, 5000)

        generator.add_source(0, 0, 9000)


        # Populate the pipeline
        for i in range(config.N_FRAMES):
            generator.add_beams()

        beam = generator.get_last_frame()

        #print(beam)
        # from python import antenna as ant

        image, points, angles = generator.mimo(beam)
        # image -= image.min()
        # image += 0.001
        # power = np.log10(image) #+ 10
        # power = -power
        # power[power < 0] = 0
        # power /= power.max()
        # power -= power.min()
        # power /= power.max()

        # print(power.min(), power.max())

        theta = angles[:,0]
        phi = angles[:,1]

        p = image[:,-1]

        power = np.log10(p) #p**0.2 #np.log10(p) #+ 10
        # power = p**0.2
        # power = p
        # power = p
        # power[power < 0] = 0
        power -= power.min()
        # power /= power.max()

        # power *= 10

        print(power.min(), power.max())

        p = power

        # p = power
        # p += 7

        x = p * np.cos(theta) * np.sin(phi)
        y = p * np.sin(theta) * np.sin(phi)
        z = p * np.cos(phi)

        # Create a 3D plot
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')

        # Plot the radiation pattern surface
        # surf = ax.plot_surface(x, y, z, cmap='viridis', rstride=1, cstride=1)

        # Create a surface plot with distance as the color value
        # surf = ax.plot_surface(x, y, z, facecolors=plt.cm.viridis(power), rstride=1, cstride=1, linewidth=0, antialiased=False)

        from matplotlib import cm, colors
        mycol = cm.jet(z.shape[0])
        colors = []

        for i in range(z.shape[0]):
            dist = np.sqrt(x[i]**2 + y[i]**2 + z[i]**2)
            colors.append(dist)
        surf = ax.scatter(x, y, z, c=colors)

        # surf = ax.plot_surface(x_grid, y_grid, z_grid, cmap='viridis')
        # Customize the plot as needed
        ax.set_title(f'Antenna Array Radiation Pattern ({config.COLUMNS}x{config.ROWS})')
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')

        # Add a colorbar to indicate distance values
        fig.colorbar(surf, shrink=0.5, aspect=5)
        plt.axis('equal')
        plt.show()

        return

        def foo():

            # Create a grid for x and y
            x_grid, y_grid = np.meshgrid(x, y)

            from scipy.interpolate import griddata

            # Interpolate z values onto the grid
            z_grid = griddata((x, y), z, (x_grid, y_grid), method='cubic') 

            # fig = plt.figure()
            # ax = fig.add_subplot(111, projection='3d')


            theta, phi = np.meshgrid(theta, phi)

            # Define your radiation pattern function (replace this with your actual function)
            # Example: a simple dipole pattern
            def radiation_pattern(theta, phi):
                return np.abs(np.cos(phi))

            # Calculate the radiation intensity at each point in the grid
            intensity = radiation_pattern(theta, phi)
            intensity = p

            # Convert polar coordinates to Cartesian coordinates
            x = intensity * np.sin(phi) * np.cos(theta)
            y = intensity * np.sin(phi) * np.sin(theta)
            z = intensity * np.cos(phi)

        def interp_array(N1):  # add interpolated rows and columns to array
            N2 = np.empty([int(N1.shape[0]), int(2*N1.shape[1] - 1)])  # insert interpolated columns
            N2[:, 0] = N1[:, 0]  # original column
            for k in range(N1.shape[1] - 1):  # loop through columns
                N2[:, 2*k+1] = np.mean(N1[:, [k, k + 1]], axis=1)  # interpolated column
                N2[:, 2*k+2] = N1[:, k+1]  # original column
            N3 = np.empty([int(2*N2.shape[0]-1), int(N2.shape[1])])  # insert interpolated columns
            N3[0] = N2[0]  # original row
            for k in range(N2.shape[0] - 1):  # loop through rows
                N3[2*k+1] = np.mean(N2[[k, k + 1]], axis=0)  # interpolated row
                N3[2*k+2] = N2[k+1]  # original row
            return N3
        
        # print(np.degrees(theta))

        X, Y = np.meshgrid(x, y)
        R = np.sqrt(X**2 + Y**2)
        Z = z * R

        # Create a 3D plot
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')


        surf = ax.plot_surface(X, Y, Z, rstride=1, cstride=1,
        linewidth=0, antialiased=False)

        plt.axis('equal')
        plt.show()

        return

        theta += np.pi
        
        THETA, PHI = np.meshgrid(theta, phi)

        interp_factor = 3  # 0 = no interpolation, 1 = 2x the points, 2 = 4x the points, 3 = 8x, etc
        R = p
        X = R * np.sin(THETA) * np.cos(PHI)
        Y = R * np.sin(THETA) * np.sin(PHI)
        Z = R * np.cos(THETA)

        for counter in range(interp_factor):  # Interpolate between points to increase number of faces
            X = interp_array(X)
            Y = interp_array(Y)
            Z = interp_array(Z)

        fig = plt.figure()

        ax = fig.add_subplot(1,1,1, projection='3d')
        ax.grid(True)
        ax.axis('on')
        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_zticks([])

        N_FRAMES = np.sqrt(X**2 + Y**2 + Z**2)
        Rmax = np.max(N_FRAMES)
        N_FRAMES = N_FRAMES/Rmax

        axes_length = 1.5
        ax.plot([0, axes_length*Rmax], [0, 0], [0, 0], linewidth=2, color='red')
        ax.plot([0, 0], [0, axes_length*Rmax], [0, 0], linewidth=2, color='green')
        ax.plot([0, 0], [0, 0], [0, axes_length*Rmax], linewidth=2, color='blue')

        # Find middle points between values for face colours
        N_FRAMES = interp_array(N_FRAMES)[1::2,1::2]
        from matplotlib import cm, colors
        mycol = cm.jet(N_FRAMES)

        surf = ax.plot_surface(X, Y, Z, rstride=1, cstride=1, facecolors=mycol, linewidth=0.5, antialiased=True, shade=False)  # , alpha=0.5, zorder = 0.5)

        ax.set_xlim([-axes_length*Rmax, axes_length*Rmax])
        ax.set_ylim([-axes_length*Rmax, axes_length*Rmax])
        ax.set_zlim([-axes_length*Rmax, axes_length*Rmax])
        
        m = cm.ScalarMappable(cmap=cm.jet)
        m.set_array(R)
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')

        # fig.colorbar(m, shrink=0.8)
        ax.view_init(azim=300, elev=30)

        plt.show()

        return

        from scipy.spatial import Delaunay

        # Create a 3D plot
        fig = plt.figure(figsize=(10, 8))
        ax = fig.add_subplot(111, projection='3d')

        # Plot the radiation pattern surface
        # surf = ax.plot_surface(x, y, z, cmap='viridis', rstride=1, cstride=1)

        # Create a surface plot with distance as the color value
        # surf = ax.plot_surface(x, y, z, facecolors=plt.cm.viridis(power), rstride=1, cstride=1, linewidth=0, antialiased=False)

        surf = ax.scatter(x, y, z)

        # Perform Delaunay triangulation
        tri = Delaunay(np.c_[x, y])

        # Plot the triangulated surface
        ax.plot_trisurf(x, y, z, triangles=tri.simplices, cmap='viridis')
        # surf = ax.plot_surface(x_grid, y_grid, z_grid, cmap='viridis')
        # Customize the plot as needed
        ax.set_title(f'Antenna Array Radiation Pattern ({config.COLUMNS}x{config.ROWS})')
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')

        # Add a colorbar to indicate distance values
        fig.colorbar(surf, shrink=0.5, aspect=5)
        plt.axis('equal')
        plt.show()



    def test_mimo_generation(self):
        """Testing that generating a mimo works as expected"""

        generator = dataset.DatasetGenerator()
        print(generator.array)
        
        return
        generator = dataset.DatasetGenerator()

        # Add some sources
        #generator.add_source(0, 0, 8000)

        generator.add_source(50, 30, 8000)


        # Populate the pipeline
        for i in range(config.N_FRAMES):
            generator.add_beams()

        beam = generator.get_last_frame()

        #print(beam)

        image, _ = generator.mimo(beam)

        power = np.log10(image) #+ 10
        # power[power < 0] = 0
        power -= power.min()
        power /= power.max()


        angle = 90
        # Create a grid of azimuth and elevation values in the -90 to +90 range
        azimuth, elevation = np.meshgrid(np.linspace(-angle, angle, power.shape[1]),
                                        np.linspace(-angle, angle, power.shape[0]))

        # Convert azimuth and elevation to radians
        azimuth_rad = np.deg2rad(azimuth)
        elevation_rad = np.deg2rad(90 - elevation)  # Convert to zenith angle

        elevation_rad = np.deg2rad(90 - elevation)

        # Calculate 3D Cartesian coordinates (x, y, z) from spherical coordinates (r=1, θ=azimuth, φ=elevation)
        x = power * np.sin(elevation_rad) * np.cos(azimuth_rad)
        y = power * np.sin(elevation_rad) * np.sin(azimuth_rad)
        
        power[power < np.mean(power) * 0.1] = 0
        z = power * np.cos(elevation_rad)

        # Create a 3D plot
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        # Create a surface plot with distance as the color value
        surf = ax.plot_surface(x, y, z, facecolors=plt.cm.viridis(power), rstride=1, cstride=1, linewidth=0, antialiased=False)

        # Customize the plot as needed
        ax.set_title(f'Antenna Array Radiation Pattern ({config.COLUMNS}x{config.ROWS})')
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')

        # Add a colorbar to indicate distance values
        fig.colorbar(surf, shrink=0.5, aspect=5)

        plt.show()




    def test(self):
        """Testing dataset creation"""

        #self.test_mimo_generation()
        return



        #data2 = dataset.DatasetGenerator()
        data = dataset.DatasetGenerator()

        data.set_sine_type(1.0, 1000)

        # data.configure_beam(-45, 70, 1000)
        # data.add_source(-20, 41, 3500)
        #data.add_source(50, 0, 2900)

        #ata.add_source(30, 30, 8500)
        # data.add_source(-30, 30, 8500)
        # data.add_source(30, -30, 8500)
        data.add_source(0, 0, 8000)

        for i in range(config.N_FRAMES):
            # data.add_sine()
            # data.store_all()
            # data.add_beam()
            data.add_beams()

        # for i in range(config.N_FRAMES):
        #     data2.store_all(data.create_directed_beam(0, 0))

        # print(data.get_last_frame())

        beam = data.get_last_frame()

        # beam = data.create_directed_beam(0, 0)

        # plt.set_cmap("jet")
        # plt.rcParams['image.cmap']='jet'

        import matplotlib

        colors = matplotlib.colormaps.get_cmap("jet")

        image, this,  = data.mimo(beam)
        print(image)

        # # this = this.sum(axis)
        # this = np.log10(image[10])

        # # plt.imshow(image)
        # radians = np.linspace(np.radians(0), 2*np.pi, len(this))

        # # plt.polar(radians, this)

        # THETA, PHI = np.meshgrid(radians, radians)
        # # R = np.cos(PHI**2)
        # R = 1
        # X = R * np.sin(PHI) * np.cos(THETA)
        # Y = R * np.sin(PHI) * np.sin(THETA)

        # print(X, X.shape)

        #from mpl_toolkits.mplot3d import Axes3D

        # fig = plt.figure()
        # ax = fig.add_subplot(1,1,1, projection='3d')
        # plot = ax.plot_surface(
        #     X, Y, image,  rstride=1, cstride=1, cmap=plt.get_cmap('jet'))#,
        #     #linewidth=0, antialiased=False, alpha=0.5)
        # # plt.plot(beam)

        # image += 1
        # image *= 10

        print(np.min(image))

        # image /= np.max(image)

        # your_image_data = image

        your_image_data = np.log10(image) #+ 10
        # your_image_data[your_image_data < 0] = 0
        your_image_data -= your_image_data.min()
        # # your_image_data += 10
        your_image_data /= your_image_data.max()

        # your_image_data **= 3

        # your_image_data = image

        # # Create a grid of azimuth and elevation values
        # azimuth, elevation = np.meshgrid(np.linspace(0, 2*np.pi, your_image_data.shape[1]),
        #                                 np.linspace(0, np.pi, your_image_data.shape[0]))

        # # Convert spherical coordinates (azimuth, elevation) to Cartesian coordinates (x, y, z)
        # x = your_image_data * np.sin(elevation) * np.cos(azimuth)
        # y = your_image_data * np.sin(elevation) * np.sin(azimuth)
        # z = your_image_data * np.cos(elevation)

        # # Create a 3D plot
        # fig = plt.figure()
        # ax = fig.add_subplot(111, projection='3d')

        # # Create a surface plot with distance as the color value
        # surf = ax.plot_surface(x, y, z, facecolors=plt.cm.viridis(your_image_data), rstride=1, cstride=1, linewidth=0, antialiased=False)


        angle = 90
        # Create a grid of azimuth and elevation values in the -90 to +90 range
        azimuth, elevation = np.meshgrid(np.linspace(-angle, angle, your_image_data.shape[1]),
                                        np.linspace(-angle, angle, your_image_data.shape[0]))

        # Convert azimuth and elevation to radians
        azimuth_rad = np.deg2rad(azimuth)
        elevation_rad = np.deg2rad(90 - elevation)  # Convert to zenith angle

        # Calculate 3D Cartesian coordinates (x, y, z) from spherical coordinates (r=1, θ=azimuth, φ=elevation)
        x = your_image_data * np.sin(elevation_rad) * np.cos(azimuth_rad)
        y = your_image_data * np.sin(elevation_rad) * np.sin(azimuth_rad)
        z = your_image_data * np.cos(elevation_rad)

        # Create a 3D plot
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        # Create a surface plot with distance as the color value
        surf = ax.plot_surface(x, y, z, facecolors=plt.cm.viridis(your_image_data), rstride=1, cstride=1, linewidth=0, antialiased=False)

        # Customize the plot as needed
        ax.set_title(f'Antenna Array Radiation Pattern ({config.COLUMNS}x{config.ROWS})')
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')

        # Add a colorbar to indicate distance values
        fig.colorbar(surf, shrink=0.5, aspect=5)

        # azimuth, elevation = np.meshgrid(np.linspace(0, 2*np.pi, your_image_data.shape[1]),
        #                          np.linspace(0, np.pi, your_image_data.shape[0]))

        # # Convert spherical coordinates (azimuth, elevation) to Cartesian coordinates (x, y, z)
        # x = your_image_data * np.sin(elevation) * np.cos(azimuth)
        # y = your_image_data * np.sin(elevation) * np.sin(azimuth)
        # z = your_image_data * np.cos(elevation)

        # # Create a 3D plot
        # fig = plt.figure()
        # ax = fig.add_subplot(111, projection='3d')

        # # Plot the data points using scatter
        # ax.scatter(x, y, z, c=your_image_data, cmap='viridis', s=10)

        # for i in range(config.N_SENSORS):
        #     color = colors(i/(config.N_SENSORS-1))
        #     plt.plot(beam[i], c=color, label=f"Sensor: {i}")
        #     # if i % config.COLUMNS == 0:
        #     #     plt.plot(beam[i], label=f"Sensor: {i}")
        #     if i > 10:
        #         break

        # plt.legend()
        # plt.set_cmap("jet")
        plt.show()


        return

        data.set_sine_type(1.0, 1000)

        for i in range(config.N_FRAMES):
            data.add_sine()

        max_delay = 2

        delays = np.linspace(0, max_delay + 1, config.N_SENSORS, dtype=np.float32)

        frames = data.delay_last_frame(delays)
        for i in range(config.N_SENSORS):
            plt.plot(frames[i], label=f"Sensor: {i}")
            if i > 10:
                break

        plt.legend()
        plt.show()



