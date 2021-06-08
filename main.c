/**
 * main.c
 * Copyright (C) 2021, Yizhen Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>

#include "avr.h"
#include "lcd.h"

const char *KEYMAP[16] = {
	"1", "2", "3", "", 
	"4", "5", "6", "",
	"7", "8", "9", "",
	"*", "0", "#", "",
};

struct State
{
	unsigned char current, next, key_exists;
	char *keys[3];
};

void
play_note(float frequency, float duration)
{
	int counter = 0, k = duration / (1.0 / frequency), wt = ((1 / frequency) / 2) / 0.00005;
	while (counter < k) {
		SET_BIT(PORTB, 3);
		avr_wait2(wt);
		CLR_BIT(PORTB, 3);
		avr_wait2(wt);
		++counter;
	}
}

void
play_keypress_tone(void)
{
	play_note(493.9, 0.25);
}

void 
play_warning_tone(void)
{
	play_note(233.1, 0.25);
	play_note(246.9, 0.25);
	play_note(220, 0.25);
	play_note(233.1, 0.25);
}

void
play_lock_unlock_tone(void)
{
	play_note(87.3, 0.25);
	play_note(103.8, 0.25);
	play_note(123.5, 0.25);
}

void
play_failed_attempt_tone(void)
{
	play_note(174.6, 0.15);
	play_note(174.6, 0.15);
	play_note(164.8, 0.15);
	play_note(164.8, 0.15);
}

int
is_pressed(int r, int c)
{
	DDRC = 0;
	PORTC = 0;
	
	SET_BIT(DDRC, r);
	SET_BIT(PORTC, c + 4);
	avr_wait(1);
	return GET_BIT(PINC, c + 4) == 0;
}

int
get_key(void)
{
	int i, j;
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			if (is_pressed(i, j)) {
				return i * 4 + j + 1;
			}
		}
	}
	return 0;
}

char
detect_used_password(struct State *state, char *password)
{
	for (unsigned char position = 0; position < 3; ++position) {
		if (!strcmp(state->keys[position], password)) {
			return 1;
		}
	}
	return 0;
}

char
validate_new_password(struct State *state, char *password)
{
	return detect_used_password(state, password) == 0 && strlen(password) > 0;
}

void
append_password_string(char *password, int *current_capacity, int key)
{	
	size_t current_size = strlen(password);
	if (current_size + 1 >= *current_capacity - 1) {
		*current_capacity = (*current_capacity * 2) + 1;
		realloc(password, *current_capacity);
	}
	strcat(password, KEYMAP[key - 1]);
}

void
enter_set_key_screen(struct State *state)
{
	lcd_print("Enter the Key.", "A)OK");
	play_keypress_tone();
	int current_capacity = 101;
	char *password = (char *) calloc(current_capacity, sizeof(char));

	char is_running = 1;
	while (is_running) {
		int key = get_key();
		switch (key) {
			case 4:
				play_keypress_tone();
				if (validate_new_password(state, password)) {
					state->current = state->next;
					state->next = (state->next + 1) % 3;
					free(state->keys[state->current]);
					state->keys[state->current] = strcpy((char *) malloc(sizeof(char) * strlen(password)), password);
					state->key_exists = 1;
					is_running = 0;
					play_lock_unlock_tone();
				} else {
					lcd_print("Try Again.", "A)OK");
					play_warning_tone();
					free(password);
					password = (char *) calloc(current_capacity, sizeof(char));
				}
				break;
			case 1: case 2: case 3: case 5: case 6: case 7: case 9: case 10: case 11: case 14:
				play_keypress_tone();
				append_password_string(password, &current_capacity, key);
				break;
			default:
				break;
		}
		avr_wait(50);
	}

	free(password);
}

int
validate_password(struct State *state, char *password)
{
	return strcmp(state->keys[state->current], password);
}

void
enter_lock_screen(struct State *state)
{
	if (state->key_exists) {
		lcd_print("Enter the Key.", "A)OK");
		play_lock_unlock_tone();
		int current_capacity = 101, total_attempts = 0;
		char *password = (char *) calloc(current_capacity, sizeof(char));

		char is_running = 1;
		while (is_running) {
			int key = get_key();
			switch (key) {
				case 4:
					if (validate_password(state, password)) {
						++total_attempts;
						if (total_attempts >= 3) {
							total_attempts = 0;
							lcd_print("Lock out for 60", "seconds.");
							play_failed_attempt_tone();	
							avr_wait(60000);
							lcd_print("Try Again.", "A)OK");
						} else {
							lcd_print("Try Again.", "A)OK");
							play_warning_tone();
						}
						free(password);
						password = (char *) calloc(current_capacity, sizeof(char));
					} else {
						play_lock_unlock_tone();
						state->key_exists = 0;
						is_running = 0;
					}
					break;
				case 1: case 2: case 3: case 5: case 6: case 7: case 9: case 10: case 11: case 14:
					play_keypress_tone();
					append_password_string(password, &current_capacity, key);
					break;
				default:
					break;
			}
			avr_wait(100);
		}

		free(password);
	} else {
		lcd_print("No Password Set.", "");
		play_warning_tone();
	}
}

int
main(void)
{
	lcd_init();
	SET_BIT(DDRB, 3);
	struct State state = { 
		0, 
		0, 
		0, 
		{ 
			(char *) calloc(101, sizeof(char)), 
			(char *) calloc(101, sizeof(char)), 
			(char *) calloc(101, sizeof(char)) 
		}
	};

	while (1) {
		lcd_print("CGuard v0.1", "A)SET B)LOCK");
		int key = get_key();
		switch (key) {
			case 4:
				enter_set_key_screen(&state);
				break;
			case 8:
				enter_lock_screen(&state);
				break;
			default:
				break;
		}
		avr_wait(200);
	}
}
