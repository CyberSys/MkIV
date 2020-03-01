#!/bin/bash
for i in {bags.mesh bakshrub_fir.mesh barrel.mesh belgian-gate.mesh Bush.mesh crate.mesh czech-hog.mesh fir05_30.mesh fir06_30.mesh fir14_25.mesh FirTree3.mesh footPath.mesh fountain.mesh grave.mesh hog2.mesh jeep.mesh nebel.mesh outhouse.mesh Pine1.mesh rampobstacle.mesh seawall.mesh spookyLamp.mesh teller-pole.mesh tree2.mesh tree.mesh trenchfence.mesh well.mesh wire.mesh}
do
 OgreMeshUpgrade -b $i
done
