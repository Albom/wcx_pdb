# wcx_pdb
A WCX plugin for Total Commander (or Double Commander). 

WCX_PDB allows you to open (Ctrl+PageDown) Palm DB files (*.pdb), 
view records (F3), extract the content (F5), and create new databases (Alt+F5).

Language is set automatically by using Windows' locale. 
English, russian and ukrainian languages are present now.

ZLIB1.DLL is needed for zTXT format.

Converting of the content to PC formats is supported. 
Converting is applied for types:
 - Foto (id: Foto) -> jpg
 - DATA (id: MNBi) -> txt
 - TEXt (id: REAd) -> txt
 - zTXT (id: GPlm) -> txt

Creating (Alt+F5) a Palm DB is possible from any file with text information:
 - file (text) -> TEXt (id: REAd)
