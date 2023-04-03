import plotly.graph_objects as go

def create_roi(x, y, z, size_x, size_y, size_z, color):
  xp = x + size_x
  yp = y + size_y
  zp = z + size_z

  return go.Mesh3d(
      x=[x, xp, xp, x, x, xp, xp, x],
      y=[y, y, yp, yp, y, y, yp, yp],
      z=[z, z, z, z, zp, zp, zp, zp],

      i=[7, 0, 0, 0, 4, 4, 6, 6, 4, 0, 3, 2],
      j=[3, 4, 1, 2, 5, 6, 5, 2, 0, 1, 6, 3],
      k=[0, 7, 2, 3, 6, 7, 1, 1, 5, 5, 7, 6],
      opacity=0.6,
      color=color
  )
