import KratosMultiphysics
import python_process
import math

##all the processes python processes should be derived from "python_process"
class IGAApplySupportIncremental(python_process.PythonProcess):
    def __init__(self, model_part, variables, mesh_id=0 ):
        python_process.PythonProcess.__init__(self) 
        self.model_part = model_part
        for condition in model_part.Conditions:
            for variable_key in variables:
                condition.SetValue(eval(variable_key), variables[variable_key])

        print("Finished construction of IGAApplySupportIncremental Process")
        
    def ExecuteInitialize(self):
        pass
    
    def ExecuteBeforeSolutionLoop(self):
        pass
    
    def ExecuteInitializeSolutionStep(self):
        #pass
        current_time = self.model_part.ProcessInfo[KratosMultiphysics.TIME]
        for condition in self.model_part.Conditions:
            displacements = KratosMultiphysics.Vector(3)
            displacements = condition.GetValue(KratosMultiphysics.DISPLACEMENT)
            #disp_y = condition.GetValue(KratosMultiphysics.DISPLACEMENT_Y)
            #disp_z = condition.GetValue(KratosMultiphysics.DISPLACEMENT_Z)
            #if (current_time<0.3):
            displacements[0] = current_time
            #displacements[1] = 0.1*current_time

            condition.SetValue(KratosMultiphysics.DISPLACEMENT, displacements)		
            #condition.SetValue(KratosMultiphysics.DISPLACEMENT_Y, disp_y + current_time)		
            #condition.SetValue(KratosMultiphysics.DISPLACEMENT_Z, disp_z + current_time)	

    def ExecuteFinalizeSolutionStep(self):
        pass
    
    def ExecuteBeforeOutputStep(self):
        pass
    
    def ExecuteAfterOutputStep(self):
        pass
        
    def ExecuteFinalize(self):
        pass
    
    def Clear(self):
        pass


    
    
def Factory(settings, Model):
	params = settings["parameters"]
	#print(params["model_part_name"].GetString())
	model_part_parent = Model[params["model_part_name"].GetString()]
	if(model_part_parent.HasSubModelPart(params["sub_model_part_name"].GetString())):
		model_part = model_part_parent.GetSubModelPart(params["sub_model_part_name"].GetString())# , "model part not found" )
		mesh_id = params["mesh_id"]

		variables = {}
		#if(settings["process_name"] == "IGAApplyLoad"):
		for variable_i in range (0,params["variables"].size()):
			variable_name = params["variables"][variable_i]["variable_name"].GetString()
			if (variable_name == "PENALTY_FACTOR"):
				variables.update({variable_name : params["variables"][variable_i]["variable"].GetDouble()})
			if (variable_name == "DISPLACEMENT_ROTATION_FIX"):
				DisplacementRotationFix = 0 #defined by rot, dispx, dispy, dispz
				if (params["variables"][variable_i]["variable"]["C1-Continuity"]["t1"].GetBool()):
					DisplacementRotationFix += 1000
				if (params["variables"][variable_i]["variable"]["C0-Continuity"]["x"].GetBool()):
					DisplacementRotationFix += 100
				if (params["variables"][variable_i]["variable"]["C0-Continuity"]["y"].GetBool()):
					DisplacementRotationFix += 10
				if (params["variables"][variable_i]["variable"]["C0-Continuity"]["z"].GetBool()):
					DisplacementRotationFix += 1
				variables.update({variable_name : DisplacementRotationFix})
			if (variable_name == "DISPLACEMENT"):
				displacements = KratosMultiphysics.Vector(3)
				displacements[0] = params["variables"][variable_i]["variable"]["C0-Continuity"]["x"].GetDouble()
				displacements[1] = params["variables"][variable_i]["variable"]["C0-Continuity"]["y"].GetDouble()
				displacements[2] = params["variables"][variable_i]["variable"]["C0-Continuity"]["z"].GetDouble()
				variables.update({variable_name : displacements})

	
		return IGAApplySupportIncremental(model_part, variables)