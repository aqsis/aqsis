import re
import xml.dom.minidom


h=open('ri.h', 'r')
f=h.read()

patt1 = re.compile(r'(// \+\+CACHE\+\+$)(.*)(// --CACHE--$)', re.MULTILINE | re.DOTALL)
patt2 = re.compile(r"""
	(Rt[a-zA-Z*]+)		# Match the return type
	\s			# Space between the return type and the function name
	(Ri[a-zA-Z]+)		# Function name
	\( \s* 			# Open bracket
	([^\)]*?)		# Arguments string for later processing
	\s* \);			# Close bracket
	(?!\s*/\*NOCACHE\*/)	# Don't match if it is followed by /*NOCACHE*/
		    """, re.MULTILINE | re.DOTALL | re.VERBOSE)
patt3 = re.compile(r"""
	\s*		# Skip whitespace
	(		# Start a group so that findall returns only this part
	    [^,]+	# Match the longest run of non ',' characters
	),?		# Followed by an optional ','
	""", re.MULTILINE | re.VERBOSE | re.DOTALL )

mm = patt1.search(f)
itf = mm.group(2)

objects = []
if( itf ):
	funcs=patt2.findall(itf)
	if funcs:
		for func in funcs:
			object={'name': func[1], 'return':func[0], 'arguments': [] }
			args = patt3.findall(func[2])
			index = 0
			for arg in args:
				if arg=='PARAMETERLIST':
					argument = {'type':'RtInt', 'name':'count'}
					object['arguments'].append(argument)
					argument = {'type':'RtTokenArray', 'name':'tokens'}
					object['arguments'].append(argument)
					argument = {'type':'RtPointerArray', 'name':'values'}
					object['arguments'].append(argument)
				elif arg=='void':
					continue
				else:
					argst = re.match('(.*?)([^ \*]*)$', arg)
					type = argst.group(1).strip()
					name = argst.group(2).strip()
					if type=='const char *':
						type = 'RtString'
					elif type=='const char **':
						type = 'RtStringArray'
					elif name.find('[]')!=-1 or name.find('*')!=-1:
						name = name.rstrip('[]')
						name = name.rstrip('*')
						name = name.strip()
						type+='Array'
					argument = {'type':type, 'name':name}
					object['arguments'].append(argument)
				index+=1
			objects.append(object)
		
impl = xml.dom.minidom.getDOMImplementation()

doc = impl.createDocument(None, "RiAPI", None)
proclist = doc.createElement("Procedures")
doc.documentElement.appendChild(proclist)
for obj in objects:
	proc = doc.createElement("Procedure")
	proc.setAttribute("name", obj['name'])
	proc.setAttribute("return", obj['return'])
	arguments = doc.createElement("Arguments")
	proc.appendChild(arguments)
	for arg in obj['arguments']:
		argument = doc.createElement("Argument")
		argument.setAttribute("name", arg['name'])
		argument.setAttribute("type", arg['type'])
		arguments.appendChild(argument)
	proclist.appendChild(proc)

print doc.toprettyxml()

