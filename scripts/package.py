from pathlib import Path
import shutil, sys, subprocess, hashlib
root=Path(__file__).resolve().parents[1]
platform=sys.argv[1]
out=root/'dist';out.mkdir(exist_ok=True)
stage=out/('ODE-TO-GSX-0.2.0-'+platform);stage.mkdir(exist_ok=True)
art=root/'build-release'/'OdeToGSX_artefacts'/'Release'
patterns=['Standalone/*.app','VST3/*.vst3','AU/*.component'] if platform=='macOS-arm64' else ['Standalone/*.exe','VST3/*.vst3']
for pattern in patterns:
 matches=list(art.glob(pattern))
 if not matches: raise RuntimeError('Missing '+pattern)
 for src in matches:
  dst=stage/src.name
  if src.is_dir(): shutil.copytree(src,dst,symlinks=True)
  else: shutil.copy2(src,dst)
for name in ['LICENSE','DISTRIBUTION.txt','DEPENDENCIES.txt']:
 shutil.copy2(root/name,stage/name)
shutil.copytree(root/'LICENSES',stage/'LICENSES')
if platform=='macOS-arm64':
 for bundle in list(stage.glob('*.app'))+list(stage.glob('*.vst3'))+list(stage.glob('*.component')):
  subprocess.run(['codesign','--force','--deep','--sign','-',str(bundle)],check=True)
  subprocess.run(['codesign','--verify','--deep','--strict',str(bundle)],check=True)
 subprocess.run(['ditto','-c','-k','--sequesterRsrc','--keepParent',str(stage),str(stage)+'.zip'],check=True)
else: shutil.make_archive(str(stage),'zip',stage.parent,stage.name)
archive=Path(str(stage)+'.zip')
archive.with_suffix('.sha256').write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+archive.name+'\n')
