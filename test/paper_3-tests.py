#!/usr/bin/env python
from pathlib import Path

def get_results_dir(base_path):
  candidate_path = base_path / 'results/'
  return candidate_path if candidate_path.is_dir() else None
  #candidates = list(path.glob('./results/'))
  #return candidates[0] if len(candidates) > 0 else None

def find_results_dir():
  """Recursively find the results directory, starting from the current working
  directory, including parent directories.

  Returns a the path pointing to the results directory.
  """
  results_dir = None
  cur_dir = Path.cwd()
  parent_dirs = list(cur_dir.parents)

  parent_dirs.reverse()

  while results_dir := get_results_dir(cur_dir) is None:
    cur_dir = parent_dirs.pop()



  candidates = list(cur_dir.glob('./results/'))
  if len(candidates) == 0:
    for p in



  while results_dir is None:


    else:
      results_dir = candidates[0]

  return results_dir

if __name__ == "__main__":
  x = find_results_dir()
  print(x)
