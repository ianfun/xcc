Token getKeyword(const char *s){
	switch(*s++){
		case 'e':
		switch(*s++){
			case 'x':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'n':
							switch(*s++){
								case '\0': return Kextern;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'n':
			switch(*s++){
				case 'u':
				switch(*s++){
					case 'm':
					switch(*s++){
						case '\0': return Kenum;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'l':
			switch(*s++){
				case 's':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return Kelse;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 's':
		switch(*s++){
			case 'h':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 't':
						switch(*s++){
							case '\0': return Kshort;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 't':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'c':
							switch(*s++){
								case '\0': return Kstatic;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'r':
				switch(*s++){
					case 'u':
					switch(*s++){
						case 'c':
						switch(*s++){
							case 't':
							switch(*s++){
								case '\0': return Kstruct;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'i':
			switch(*s++){
				case 'g':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'd':
							switch(*s++){
								case '\0': return Ksigned;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'z':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'o':
						switch(*s++){
							case 'f':
							switch(*s++){
								case '\0': return Ksizeof;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'w':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'c':
						switch(*s++){
							case 'h':
							switch(*s++){
								case '\0': return Kswitch;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'r':
		switch(*s++){
			case 'e':
			switch(*s++){
				case 'g':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 's':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'r':
									switch(*s++){
										case '\0': return Kregister;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 's':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case 't':
									switch(*s++){
										case '\0': return Krestrict;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'u':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'n':
							switch(*s++){
								case '\0': return Kreturn;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case '_':
		switch(*s++){
			case 'T':
			switch(*s++){
				case 'h':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'a':
							switch(*s++){
								case 'd':
								switch(*s++){
									case '_':
									switch(*s++){
										case 'l':
										switch(*s++){
											case 'o':
											switch(*s++){
												case 'c':
												switch(*s++){
													case 'a':
													switch(*s++){
														case 'l':
														switch(*s++){
															case '\0': return K_Thread_local;
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'N':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'u':
								switch(*s++){
									case 'r':
									switch(*s++){
										case 'n':
										switch(*s++){
											case '\0': return K_Noreturn;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'D':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'c':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'm':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'l':
									switch(*s++){
										case '3':
										switch(*s++){
											case '2':
											switch(*s++){
												case '\0': return K_Decimal32;
												default: return TNul;
											}
											default: return TNul;
										}
										case '1':
										switch(*s++){
											case '2':
											switch(*s++){
												case '8':
												switch(*s++){
													case '\0': return K_Decimal128;
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										case '6':
										switch(*s++){
											case '4':
											switch(*s++){
												case '\0': return K_Decimal64;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case '_':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 't':
						switch(*s++){
							case '1':
							switch(*s++){
								case '2':
								switch(*s++){
									case '8':
									switch(*s++){
										case '\0': return K__int128;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 'b':
					switch(*s++){
						case 'm':
						switch(*s++){
							case '1':
							switch(*s++){
								case '2':
								switch(*s++){
									case '8':
									switch(*s++){
										case '\0': return K__ibm128;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 'm':
					switch(*s++){
						case 'a':
						switch(*s++){
							case 'g':
							switch(*s++){
								case '\0': return K__imag;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'f':
				switch(*s++){
					case 'l':
					switch(*s++){
						case 'o':
						switch(*s++){
							case 'a':
							switch(*s++){
								case 't':
								switch(*s++){
									case '1':
									switch(*s++){
										case '2':
										switch(*s++){
											case '8':
											switch(*s++){
												case '\0': return K__float128;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									case '8':
									switch(*s++){
										case '0':
										switch(*s++){
											case '\0': return K__float80;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 'u':
					switch(*s++){
						case 'n':
						switch(*s++){
							case 'c':
							switch(*s++){
								case '_':
								switch(*s++){
									case '_':
									switch(*s++){
										case '\0': return K__func__;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 'a':
					switch(*s++){
						case 's':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'c':
								switch(*s++){
									case 'a':
									switch(*s++){
										case 'l':
										switch(*s++){
											case 'l':
											switch(*s++){
												case '\0': return K__fastcall;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'b':
				switch(*s++){
					case 'f':
					switch(*s++){
						case '1':
						switch(*s++){
							case '6':
							switch(*s++){
								case '\0': return K__bf16;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 'u':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 't':
								switch(*s++){
									case 'i':
									switch(*s++){
										case 'n':
										switch(*s++){
											case '_':
											switch(*s++){
												case 'c':
												switch(*s++){
													case 'h':
													switch(*s++){
														case 'o':
														switch(*s++){
															case 'o':
															switch(*s++){
																case 's':
																switch(*s++){
																	case 'e':
																	switch(*s++){
																		case '_':
																		switch(*s++){
																			case 'e':
																			switch(*s++){
																				case 'x':
																				switch(*s++){
																					case 'p':
																					switch(*s++){
																						case 'r':
																						switch(*s++){
																							case '\0': return K__builtin_choose_expr;
																							default: return TNul;
																						}
																						default: return TNul;
																					}
																					default: return TNul;
																				}
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'o':
												switch(*s++){
													case 'f':
													switch(*s++){
														case 'f':
														switch(*s++){
															case 's':
															switch(*s++){
																case 'e':
																switch(*s++){
																	case 't':
																	switch(*s++){
																		case 'o':
																		switch(*s++){
																			case 'f':
																			switch(*s++){
																				case '\0': return K__builtin_offsetof;
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'F':
												switch(*s++){
													case 'I':
													switch(*s++){
														case 'L':
														switch(*s++){
															case 'E':
															switch(*s++){
																case '\0': return K__builtin_FILE;
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													case 'U':
													switch(*s++){
														case 'N':
														switch(*s++){
															case 'C':
															switch(*s++){
																case 'T':
																switch(*s++){
																	case 'I':
																	switch(*s++){
																		case 'O':
																		switch(*s++){
																			case 'N':
																			switch(*s++){
																				case '\0': return K__builtin_FUNCTION;
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'L':
												switch(*s++){
													case 'I':
													switch(*s++){
														case 'N':
														switch(*s++){
															case 'E':
															switch(*s++){
																case '\0': return K__builtin_LINE;
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'C':
												switch(*s++){
													case 'O':
													switch(*s++){
														case 'L':
														switch(*s++){
															case 'U':
															switch(*s++){
																case 'M':
																switch(*s++){
																	case 'N':
																	switch(*s++){
																		case '\0': return K__builtin_COLUMN;
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 't':
												switch(*s++){
													case 'y':
													switch(*s++){
														case 'p':
														switch(*s++){
															case 'e':
															switch(*s++){
																case 's':
																switch(*s++){
																	case '_':
																	switch(*s++){
																		case 'c':
																		switch(*s++){
																			case 'o':
																			switch(*s++){
																				case 'm':
																				switch(*s++){
																					case 'p':
																					switch(*s++){
																						case 'a':
																						switch(*s++){
																							case 't':
																							switch(*s++){
																								case 'i':
																								switch(*s++){
																									case 'b':
																									switch(*s++){
																										case 'l':
																										switch(*s++){
																											case 'e':
																											switch(*s++){
																												case '_':
																												switch(*s++){
																													case 'p':
																													switch(*s++){
																														case '\0': return K__builtin_types_compatible_p;
																														default: return TNul;
																													}
																													default: return TNul;
																												}
																												default: return TNul;
																											}
																											default: return TNul;
																										}
																										default: return TNul;
																									}
																									default: return TNul;
																								}
																								default: return TNul;
																							}
																							default: return TNul;
																						}
																						default: return TNul;
																					}
																					default: return TNul;
																				}
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'v':
												switch(*s++){
													case 'a':
													switch(*s++){
														case '_':
														switch(*s++){
															case 'a':
															switch(*s++){
																case 'r':
																switch(*s++){
																	case 'g':
																	switch(*s++){
																		case '\0': return K__builtin_va_arg;
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'b':
												switch(*s++){
													case 'i':
													switch(*s++){
														case 't':
														switch(*s++){
															case '_':
															switch(*s++){
																case 'c':
																switch(*s++){
																	case 'a':
																	switch(*s++){
																		case 's':
																		switch(*s++){
																			case 't':
																			switch(*s++){
																				case '\0': return K__builtin_bit_cast;
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												case 'a':
												switch(*s++){
													case 'v':
													switch(*s++){
														case 'a':
														switch(*s++){
															case 'i':
															switch(*s++){
																case 'l':
																switch(*s++){
																	case 'a':
																	switch(*s++){
																		case 'b':
																		switch(*s++){
																			case 'l':
																			switch(*s++){
																				case 'e':
																				switch(*s++){
																					case '\0': return K__builtin_available;
																					default: return TNul;
																				}
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'h':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'd':
									switch(*s++){
										case '\0': return K__thread;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						case 'i':
						switch(*s++){
							case 's':
							switch(*s++){
								case 'c':
								switch(*s++){
									case 'a':
									switch(*s++){
										case 'l':
										switch(*s++){
											case 'l':
											switch(*s++){
												case '\0': return K__thiscall;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'a':
				switch(*s++){
					case 'u':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'o':
							switch(*s++){
								case '_':
								switch(*s++){
									case 't':
									switch(*s++){
										case 'y':
										switch(*s++){
											case 'p':
											switch(*s++){
												case 'e':
												switch(*s++){
													case '\0': return K__auto_type;
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 't':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'r':
							switch(*s++){
								case 'i':
								switch(*s++){
									case 'b':
									switch(*s++){
										case 'u':
										switch(*s++){
											case 't':
											switch(*s++){
												case 'e':
												switch(*s++){
													case '\0': return K__attribute;
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'd':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'c':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 's':
								switch(*s++){
									case 'p':
									switch(*s++){
										case 'e':
										switch(*s++){
											case 'c':
											switch(*s++){
												case '\0': return K__declspec;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'c':
				switch(*s++){
					case 'd':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'c':
							switch(*s++){
								case 'l':
								switch(*s++){
									case '\0': return K__cdecl;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 's':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'd':
						switch(*s++){
							case 'c':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'l':
									switch(*s++){
										case 'l':
										switch(*s++){
											case '\0': return K__stdcall;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'r':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'c':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'l':
									switch(*s++){
										case 'l':
										switch(*s++){
											case '\0': return K__regcall;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						case 'a':
						switch(*s++){
							case 'l':
							switch(*s++){
								case '\0': return K__real;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'v':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'c':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'o':
								switch(*s++){
									case 'r':
									switch(*s++){
										case 'c':
										switch(*s++){
											case 'a':
											switch(*s++){
												case 'l':
												switch(*s++){
													case 'l':
													switch(*s++){
														case '\0': return K__vectorcall;
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'p':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 's':
						switch(*s++){
							case 'c':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'l':
									switch(*s++){
										case '\0': return K__pascal;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'P':
				switch(*s++){
					case 'R':
					switch(*s++){
						case 'E':
						switch(*s++){
							case 'T':
							switch(*s++){
								case 'T':
								switch(*s++){
									case 'Y':
									switch(*s++){
										case '_':
										switch(*s++){
											case 'F':
											switch(*s++){
												case 'U':
												switch(*s++){
													case 'N':
													switch(*s++){
														case 'C':
														switch(*s++){
															case 'T':
															switch(*s++){
																case 'I':
																switch(*s++){
																	case 'O':
																	switch(*s++){
																		case 'N':
																		switch(*s++){
																			case '_':
																			switch(*s++){
																				case '_':
																				switch(*s++){
																					case '\0': return K__PRETTY_FUNCTION__;
																					default: return TNul;
																				}
																				default: return TNul;
																			}
																			default: return TNul;
																		}
																		default: return TNul;
																	}
																	default: return TNul;
																}
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'e':
				switch(*s++){
					case 'x':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'n':
								switch(*s++){
									case 's':
									switch(*s++){
										case 'i':
										switch(*s++){
											case 'o':
											switch(*s++){
												case 'n':
												switch(*s++){
													case '_':
													switch(*s++){
														case '_':
														switch(*s++){
															case '\0': return K__extension__;
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'l':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'b':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'l':
								switch(*s++){
									case '_':
									switch(*s++){
										case '_':
										switch(*s++){
											case '\0': return K__label__;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'o':
				switch(*s++){
					case 'b':
					switch(*s++){
						case 'j':
						switch(*s++){
							case 'c':
							switch(*s++){
								case '_':
								switch(*s++){
									case 'y':
									switch(*s++){
										case 'e':
										switch(*s++){
											case 's':
											switch(*s++){
												case '\0': return K__objc_yes;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									case 'n':
									switch(*s++){
										case 'o':
										switch(*s++){
											case '\0': return K__objc_no;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'F':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'a':
						switch(*s++){
							case 't':
							switch(*s++){
								case '1':
								switch(*s++){
									case '6':
									switch(*s++){
										case '\0': return K_Float16;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'r':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'c':
						switch(*s++){
							case 't':
							switch(*s++){
								case '\0': return K_Fract;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'A':
			switch(*s++){
				case 'c':
				switch(*s++){
					case 'c':
					switch(*s++){
						case 'u':
						switch(*s++){
							case 'm':
							switch(*s++){
								case '\0': return K_Accum;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'm':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case '\0': return K_Atomic;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'l':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 's':
									switch(*s++){
										case '\0': return K_Alignas;
										default: return TNul;
									}
									default: return TNul;
								}
								case 'o':
								switch(*s++){
									case 'f':
									switch(*s++){
										case '\0': return K_Alignof;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'S':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 't':
					switch(*s++){
						case '\0': return K_Sat;
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case '_':
									switch(*s++){
										case 'a':
										switch(*s++){
											case 's':
											switch(*s++){
												case 's':
												switch(*s++){
													case 'e':
													switch(*s++){
														case 'r':
														switch(*s++){
															case 't':
															switch(*s++){
																case '\0': return K_Static_assert;
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'I':
			switch(*s++){
				case 'm':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'n':
								switch(*s++){
									case 'a':
									switch(*s++){
										case 'r':
										switch(*s++){
											case 'y':
											switch(*s++){
												case '\0': return K_Imaginary;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'C':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'm':
					switch(*s++){
						case 'p':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'x':
									switch(*s++){
										case '\0': return K_Complex;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'B':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'l':
						switch(*s++){
							case '\0': return K_Bool;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'i':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'I':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 't':
								switch(*s++){
									case '\0': return K_BitInt;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'G':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'r':
							switch(*s++){
								case 'i':
								switch(*s++){
									case 'c':
									switch(*s++){
										case '\0': return K_Generic;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'a':
		switch(*s++){
			case 'u':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case '\0': return Kauto;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 's':
			switch(*s++){
				case 'm':
				switch(*s++){
					case '\0': return Kasm;
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'c':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'n':
				switch(*s++){
					case 's':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'x':
								switch(*s++){
									case 'p':
									switch(*s++){
										case 'r':
										switch(*s++){
											case '\0': return Kconstexpr;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							case '\0': return Kconst;
							default: return TNul;
						}
						default: return TNul;
					}
					case 't':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'u':
								switch(*s++){
									case 'e':
									switch(*s++){
										case '\0': return Kcontinue;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'h':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 'r':
					switch(*s++){
						case '\0': return Kchar;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'a':
			switch(*s++){
				case 's':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return Kcase;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'i':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'n':
						switch(*s++){
							case 'e':
							switch(*s++){
								case '\0': return Kinline;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case '\0': return Kint;
					default: return TNul;
				}
				default: return TNul;
			}
			case 'f':
			switch(*s++){
				case '\0': return Kif;
				default: return TNul;
			}
			default: return TNul;
		}
		case 't':
		switch(*s++){
			case 'y':
			switch(*s++){
				case 'p':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'd':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'f':
								switch(*s++){
									case '\0': return Ktypedef;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						case 'o':
						switch(*s++){
							case 'f':
							switch(*s++){
								case '_':
								switch(*s++){
									case 'u':
									switch(*s++){
										case 'n':
										switch(*s++){
											case 'q':
											switch(*s++){
												case 'u':
												switch(*s++){
													case 'a':
													switch(*s++){
														case 'l':
														switch(*s++){
															case '\0': return Ktypeof_unqual;
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								case '\0': return Ktypeof;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'r':
			switch(*s++){
				case 'u':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return Ktrue;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'h':
		switch(*s++){
			case 'a':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'f':
					switch(*s++){
						case '\0': return Khalf;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'u':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'n':
						switch(*s++){
							case '\0': return Kunion;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 's':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'd':
									switch(*s++){
										case '\0': return Kunsigned;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'v':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'd':
					switch(*s++){
						case '\0': return Kvoid;
						default: return TNul;
					}
					default: return TNul;
				}
				case 'l':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'l':
								switch(*s++){
									case 'e':
									switch(*s++){
										case '\0': return Kvolatile;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'l':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'n':
				switch(*s++){
					case 'g':
					switch(*s++){
						case '\0': return Klong;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'f':
		switch(*s++){
			case 'l':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case '\0': return Kfloat;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'a':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 's':
					switch(*s++){
						case 'e':
						switch(*s++){
							case '\0': return Kfalse;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'o':
			switch(*s++){
				case 'r':
				switch(*s++){
					case '\0': return Kfor;
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'd':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'u':
				switch(*s++){
					case 'b':
					switch(*s++){
						case 'l':
						switch(*s++){
							case 'e':
							switch(*s++){
								case '\0': return Kdouble;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case '\0': return Kdo;
				default: return TNul;
			}
			case 'e':
			switch(*s++){
				case 'f':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'u':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 't':
								switch(*s++){
									case '\0': return Kdefault;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'n':
		switch(*s++){
			case 'u':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'l':
					switch(*s++){
						case 'p':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'r':
								switch(*s++){
									case '\0': return Knullptr;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'g':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case '\0': return Kgoto;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'b':
		switch(*s++){
			case 'r':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'k':
						switch(*s++){
							case '\0': return Kbreak;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'w':
		switch(*s++){
			case 'h':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'l':
					switch(*s++){
						case 'e':
						switch(*s++){
							case '\0': return Kwhile;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		default: return TNul;
	}
}
