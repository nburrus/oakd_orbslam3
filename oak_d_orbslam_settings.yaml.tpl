%YAML:1.0

File.version: "1.0"

#--------------------------------------------------------------------------------------------
# Camera Parameters. Adjust them!
#--------------------------------------------------------------------------------------------
Camera.type: "Rectified"

# Left Camera calibration and distortion parameters (OpenCV)
Camera1.fx: {{ cam1_fx }}
Camera1.fy: {{ cam1_fy }}
Camera1.cx: {{ cam1_cx }}
Camera1.cy: {{ cam1_cy }}

# Kannala-Brandt distortion parameters
# Camera1.k1: {{ cam1_k1 }}
# Camera1.k2: {{ cam1_k2 }}
# Camera1.p1: {{ cam1_p1 }}
# Camera1.p2: {{ cam1_p2 }}
# Camera1.k3: {{ cam1_k3 }}

# # Right Camera calibration and distortion parameters (OpenCV)
# Camera2.fx: {{ cam2_fx }}
# Camera2.fy: {{ cam2_fy }}
# Camera2.cx: {{ cam2_cx }}
# Camera2.cy: {{ cam2_cy }}

# # Kannala-Brandt distortion parameters
# Camera2.k1: {{ cam2_k1 }}
# Camera2.k2: {{ cam2_k2 }}
# Camera2.p1: {{ cam2_p1 }}
# Camera2.p2: {{ cam2_p2 }}
# Camera2.k3: {{ cam2_k3 }}

Stereo.b: {{ baseline_meters }}

# Transformation matrix from right camera to left camera
# Stereo.T_c1_c2: !!opencv-matrix
#   rows: 4
#   cols: 4
#   dt: f
#   data: Stereo.T_c1_c2

# Lapping area between images (FIXME: We must calculate)
Camera1.overlappingBegin: 0
Camera1.overlappingEnd: {{ width }}

Camera2.lappingBegin: 0
Camera2.lappingEnd: {{ width }}

Camera.width: {{ width }}
Camera.height: {{ height }}

# Camera frames per second 
Camera.fps: {{ fps }}

# Color order of the images (0: BGR, 1: RGB. It is ignored if images are grayscale)
Camera.RGB: 1

# Close/Far threshold. Baseline times.
Stereo.ThDepth: 35.0

#--------------------------------------------------------------------------------------------
# ORB Parameters
#--------------------------------------------------------------------------------------------

# ORB Extractor: Number of features per image
ORBextractor.nFeatures: 1200

# ORB Extractor: Scale factor between levels in the scale pyramid 	
ORBextractor.scaleFactor: 1.2

# ORB Extractor: Number of levels in the scale pyramid	
ORBextractor.nLevels: 8

# ORB Extractor: Fast threshold
# Image is divided in a grid. At each cell FAST are extracted imposing a minimum response.
# Firstly we impose iniThFAST. If no corners are detected we impose a lower value minThFAST
# You can lower these values if your images have low contrast			
ORBextractor.iniThFAST: 20
ORBextractor.minThFAST: 7

#--------------------------------------------------------------------------------------------
# Viewer Parameters
#--------------------------------------------------------------------------------------------
Viewer.KeyFrameSize: 0.05
Viewer.KeyFrameLineWidth: 1.0
Viewer.GraphLineWidth: 0.9
Viewer.PointSize: 2.0
Viewer.CameraSize: 0.08
Viewer.CameraLineWidth: 3.0
Viewer.ViewpointX: 0.0
Viewer.ViewpointY: -0.7
Viewer.ViewpointZ: -3.5
Viewer.ViewpointF: 500.0