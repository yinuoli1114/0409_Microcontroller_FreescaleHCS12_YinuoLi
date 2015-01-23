
########## Tcl recorder starts at 03/06/14 21:40:51 ##########

set version "1.5"
set proj_dir "N:/ECE362/lab6"
cd $proj_dir

# Get directory paths
set pver $version
regsub -all {\.} $pver {_} pver
set lscfile "lsc_"
append lscfile $pver ".ini"
set lsvini_dir [lindex [array get env LSC_INI_PATH] 1]
set lsvini_path [file join $lsvini_dir $lscfile]
if {[catch {set fid [open $lsvini_path]} msg]} {
	 puts "File Open Error: $lsvini_path"
	 return false
} else {set data [read $fid]; close $fid }
foreach line [split $data '\n'] { 
	set lline [string tolower $line]
	set lline [string trim $lline]
	if {[string compare $lline "\[paths\]"] == 0} { set path 1; continue}
	if {$path && [regexp {^\[} $lline]} {set path 0; break}
	if {$path && [regexp {^bin} $lline]} {set cpld_bin $line; continue}
	if {$path && [regexp {^fpgapath} $lline]} {set fpga_dir $line; continue}
	if {$path && [regexp {^fpgabinpath} $lline]} {set fpga_bin $line}}

set cpld_bin [string range $cpld_bin [expr [string first "=" $cpld_bin]+1] end]
regsub -all "\"" $cpld_bin "" cpld_bin
set cpld_bin [file join $cpld_bin]
set install_dir [string range $cpld_bin 0 [expr [string first "ispcpld" $cpld_bin]-2]]
regsub -all "\"" $install_dir "" install_dir
set install_dir [file join $install_dir]
set fpga_dir [string range $fpga_dir [expr [string first "=" $fpga_dir]+1] end]
regsub -all "\"" $fpga_dir "" fpga_dir
set fpga_dir [file join $fpga_dir]
set fpga_bin [string range $fpga_bin [expr [string first "=" $fpga_bin]+1] end]
regsub -all "\"" $fpga_bin "" fpga_bin
set fpga_bin [file join $fpga_bin]

if {[string match "*$fpga_bin;*" $env(PATH)] == 0 } {
   set env(PATH) "$fpga_bin;$env(PATH)" }

if {[string match "*$cpld_bin;*" $env(PATH)] == 0 } {
   set env(PATH) "$cpld_bin;$env(PATH)" }

lappend auto_path [file join $install_dir "ispcpld" "tcltk" "lib" "ispwidget" "runproc"]
package require runcmd

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:40:51 ###########


########## Tcl recorder starts at 03/06/14 21:46:32 ##########

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:46:32 ###########


########## Tcl recorder starts at 03/06/14 21:50:31 ##########

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:50:31 ###########


########## Tcl recorder starts at 03/06/14 21:55:20 ##########

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:55:20 ###########


########## Tcl recorder starts at 03/06/14 21:56:07 ##########

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:56:07 ###########


########## Tcl recorder starts at 03/06/14 21:56:29 ##########

# Commands to make the Process: 
# Fit Design
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -mod lab7prelab -ojhd compile -def _PLSI_ _LATTICE_  -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/iblifopt\" \"lab7prelab.bl0\" -red bypin choose -collapse -pterms 8 -family -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/iblflink\" \"lab7prelab.bl1\" -o \"lab7prelab.bl2\" -omod lab7prelab -family -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/iblifopt\" lab7prelab.bl2 -red bypin choose -sweep -collapse all -pterms 8 -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/idiofft\" lab7prelab.bl3 -pla -o lab7prelab.tt2 -dev p22v10g -define N -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/fit\" lab7prelab.tt2 -dev p22v10g -str -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:56:29 ###########


########## Tcl recorder starts at 03/06/14 21:56:54 ##########

# Commands to make the Process: 
# Compiler Listing
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -mod lab7prelab -syn -list -def _PLSI_ _LATTICE_  -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 21:56:54 ###########


########## Tcl recorder starts at 03/06/14 22:04:17 ##########

# Commands to make the Process: 
# Create Fuse Map
if [runCmd "\"$cpld_bin/fuseasm\" lab7prelab.tt3 -dev p22v10g -o lab7prelab.jed -ivec NoInput.tmv -rep lab7prelab.rpt -doc brief -con ptblown -for brief -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/synsvf\" -exe \"$install_dir/ispvmsystem/ispufw\" -prj lab7prelab -if lab7prelab.jed -j2s -log lab7prelab.svl "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/06/14 22:04:17 ###########


########## Tcl recorder starts at 03/09/14 21:08:32 ##########

# Commands to make the Process: 
# Hierarchy
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -ojhd only -def _PLSI_ _LATTICE_ -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/09/14 21:08:32 ###########


########## Tcl recorder starts at 03/09/14 21:09:13 ##########

# Commands to make the Process: 
# Compile Logic
if [runCmd "\"$cpld_bin/ahdl2blf\" \"lab7 .abl\" -mod lab7prelab -ojhd compile -def _PLSI_ _LATTICE_  -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/09/14 21:09:13 ###########


########## Tcl recorder starts at 03/09/14 21:09:24 ##########

# Commands to make the Process: 
# Fit Design
if [runCmd "\"$cpld_bin/iblifopt\" \"lab7prelab.bl0\" -red bypin choose -collapse -pterms 8 -family -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/iblflink\" \"lab7prelab.bl1\" -o \"lab7prelab.bl2\" -omod lab7prelab -family -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/iblifopt\" lab7prelab.bl2 -red bypin choose -sweep -collapse all -pterms 8 -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/idiofft\" lab7prelab.bl3 -pla -o lab7prelab.tt2 -dev p22v10g -define N -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/fit\" lab7prelab.tt2 -dev p22v10g -str -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/09/14 21:09:24 ###########


########## Tcl recorder starts at 03/09/14 21:09:29 ##########

# Commands to make the Process: 
# Create Fuse Map
if [runCmd "\"$cpld_bin/fuseasm\" lab7prelab.tt3 -dev p22v10g -o lab7prelab.jed -ivec NoInput.tmv -rep lab7prelab.rpt -doc brief -con ptblown -for brief -err automake.err "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}
if [runCmd "\"$cpld_bin/synsvf\" -exe \"$install_dir/ispvmsystem/ispufw\" -prj lab7prelab -if lab7prelab.jed -j2s -log lab7prelab.svl "] {
	return
} else {
	vwait done
	if [checkResult $done] {
		return
	}
}

########## Tcl recorder end at 03/09/14 21:09:29 ###########

