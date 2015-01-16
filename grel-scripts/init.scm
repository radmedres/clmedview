;; Load a file.
;;(grel-file-load "../testdata/test.nii")
(grel-file-load "../testdata/Frank/C001/C001_flow_magnitude.nii")
(grel-file-load "../testdata/Frank/C001/C001_flow_velocitymap.nii")
(grel-file-load "../testdata/Frank/C001/C001_flow_anatomy.nii")
(grel-file-load "../testdata/Frank/C001/C001_flow_phase.nii")

;; Load an overlay.
;;(grel-overlay-load "../testdata/test.nii")

;; Use the axial viewport as default.
(grel-set-viewport "axial")

;; Set custom keybindings
;;(grel-override-key "viewport-axial"    "u")
;;(grel-override-key "viewport-sagital"  "i")
;;(grel-override-key "viewport-coronal"  "o")
;;(grel-override-key "viewport-split"    "p")
;;(grel-override-key "toggle-grel"       " ")
