<#macro publicDecl method>public ${method.type} ${method.name}(<#list method.params as par>${par.type} ${par.name}<#sep>, </#sep></#list>)</#macro>
<#macro nativeDecl method>private native <#if method.isRetItem>int<#else>${method.type}</#if> _${method.name}(int thisID<#list method.params as par>, <#if par.isItem>int ${par.name}<#else>${par.type} ${par.name}</#if></#list>);</#macro>
<#macro nativeCall method>_${method.name}(thisID<#list method.params as par>, <#if par.isItem>${par.name}.getID()<#else>${par.name}</#if></#list>)</#macro>
package ${Item.packageName}.impl;

<#list Item.imports as import>
	import ${import};
</#list> 

public class ${Item.interfaceName}GeneratedImpl extends ${Item.superClassName} implements ${Item.packageName}.${Item.interfaceName} {
	
	protected ${Item.interfaceName}GeneratedImpl(int thisID) {
		super(thisID);
	}
 
<#list Item.methods as method>
	<@publicDecl method/> {
		<#if method.type != "void">return</#if>	<#if method.isRetItem>session.getItem(${method.type}.class, <@nativeCall method/>)<#else><@nativeCall method/></#if>;
	}
    
	<@nativeDecl method/> //
    
</#list>
}



