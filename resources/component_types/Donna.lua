Donna = {
	lives = 3,
	iframe_cooldown = 0,

	OnStart = function(self)
		self.spriterenderer = self.actor:GetComponent("SpriteRenderer")
		self.transform = self.actor:GetComponent("Transform")
	end,

	OnUpdate = function(self) 

		-- projectile spawning
		if Input.GetKey("space") and Application.GetFrame() % 10 == 0 then
			new_projectile = Actor.Instantiate("Projectile")
			new_projectile_transform = new_projectile:GetComponent("Transform")
			new_projectile_transform.x = self.transform.x
			new_projectile_transform.y = self.transform.y
		end

		-- iframes
		if self.iframe_cooldown > 0 then
			self.iframe_cooldown = self.iframe_cooldown - 1

			-- Damage flash red
			local sin_value_01 = (math.sin(Application.GetFrame() * 0.4) + 1) * 0.5
			self.spriterenderer.g = 255 * sin_value_01
			self.spriterenderer.b = 255 * sin_value_01
			self.spriterenderer.r = 255

		else
			-- Become normal color again
			self.spriterenderer.g = 255
			self.spriterenderer.b = 255
			self.spriterenderer.r = 255
		end


		-- check for collision with fireballs.
		local our_pos = {x = self.transform.x, y = self.transform.y}
		for i, fireball in ipairs(all_fireballs) do
			local fireball_transform = fireball.actor:GetComponent("Transform")
			local fireball_position = {x = fireball_transform.x, y = fireball_transform.y}
			distance = self.Distance(our_pos, fireball_position)
			if distance <= 50 then
				self:TakeDamage()
				break
			end
		end

	end,

	TakeDamage = function(self)
		if self.iframe_cooldown <= 0 then
			if game_over == false then
				Audio.Play(1, "ouch", false)
			end
			self.lives = self.lives - 1
			self.iframe_cooldown = 90
		end

		if self.lives <= 0 then
			Actor.Destroy(self.actor)
		end
	end,

	Distance = function(p1, p2)
		local dx = p1.x - p2.x
		local dy = p1.y - p2.y
		return math.sqrt(dx*dx + dy*dy)
	end,
}

