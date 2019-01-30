& secure-file\tools\secure-file -decrypt c:\projects\source\ci\streamlabsp12.pfx.enc -secret "$env:StreamlabsPfxSecret" -out c:\projects\source\ci\streamlabsp12.pfx

if ($LASTEXITCODE -ne 0) {
	exit 1
}

Get-ChildItem -Recurse  "distribute/slobs/RelWithDebInfo/" -Include "*.dll","*.node","*.exe" |
Foreach-Object {
	& "$env:SignTool" sign /as /p "$env:StreamlabsSecret" /f c:\projects\source\ci\streamlabsp12.pfx $_.FullName
	if ($LASTEXITCODE -ne 0) {
		exit 1
	}
}