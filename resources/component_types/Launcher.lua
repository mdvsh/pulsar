Launcher = {
	template_to_launch = "",
	x_vel = 1,
	y_vel = 0,
	interval_frames = 5,

	OnStart = function(self)
		self.frames_until_launch = self.interval_frames
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self)
		if self.template_to_launch == "" then
			return
		end

		self.frames_until_launch = self.frames_until_launch - 1

		if self.frames_until_launch <= 0 then
			self.frames_until_launch = self.interval_frames

			local new_actor = Actor.Instantiate(self.template_to_launch)

			local new_actor_t = new_actor:GetComponent("Transform")
			new_actor_t.x = self.transform.x
			new_actor_t.y = self.transform.y

			local new_actor_cs = new_actor:GetComponent("ConstantMovement")
			new_actor_cs.x_vel = self.x_vel
			new_actor_cs.y_vel = self.y_vel
		end
	end
}

